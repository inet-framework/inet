import datetime
import functools
import logging
import multiprocessing
import multiprocessing.pool
import sys
import time
import traceback

from inet.common.util import *

logger = logging.getLogger(__name__)

class TaskResult:
    def __init__(self, task=None, result="DONE", expected_result="DONE", reason=None, stdout=None, stderr=None, error_message=None, exception=None, elapsed_wall_time=None, possible_results=["DONE", "CANCEL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_RED], **kwargs):
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.task = task
        self.result = result
        self.expected_result = expected_result
        self.expected = expected_result == result
        self.reason = reason
        self.stdout = stdout
        self.stderr = stderr
        self.error_message = error_message
        self.exception = exception
        self.elapsed_wall_time = elapsed_wall_time
        self.possible_results = possible_results
        self.possible_result_colors = possible_result_colors
        self.color = possible_result_colors[possible_results.index(result)]

    def __repr__(self):
        return "Result: " + self.get_description()

    def get_description(self, complete_error_message=True, include_parameters=False, **kwargs):
        return (self.task.get_parameters_string() + " " if include_parameters else "") + \
                self.color + self.result + COLOR_RESET + \
                ((COLOR_YELLOW + " (unexpected)" + COLOR_RESET) if not self.expected and self.color != COLOR_GREEN else "") + \
                ((COLOR_GREEN + " (expected)" + COLOR_RESET) if self.expected and self.color != COLOR_GREEN else "") + \
               (" (" + self.reason + ")" if self.reason else "") + \
               (" " + self.get_error_message(complete_error_message=complete_error_message) if self.result == "ERROR" else "")

    def get_error_message(self, **kwargs):
        return self.error_message or self.stderr or (self.exception and str(self.exception)) or "<No error message>"

    def print_result(self, complete_error_message=False, output_stream=sys.stdout, **kwargs):
        print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

    def recreate(self, **kwargs):
        return self.__class__(**dict(dict(self.locals, **self.kwargs), **kwargs))

    def rerun(self, **kwargs):
        return self.task.rerun(**kwargs)

class MultipleTaskResults:
    def __init__(self, multiple_tasks=None, results=[], expected_result="DONE", elapsed_wall_time=None, possible_results=["DONE", "CANCEL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_RED], **kwargs):
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.multiple_tasks = multiple_tasks
        self.results = results
        self.expected_result = expected_result
        self.elapsed_wall_time = elapsed_wall_time
        self.possible_results = possible_results
        self.possible_result_colors = possible_result_colors
        self.num_different_results = 0
        self.num_expected = {}
        self.num_unexpected = {}
        for possible_result in self.possible_results:
            self.num_expected[possible_result] = self.count_results(possible_result, True)
            self.num_unexpected[possible_result] = self.count_results(possible_result, False)
        self.result = possible_results[0]
        for possible_result in self.possible_results:
            if self.num_expected[possible_result] != 0:
                self.result = possible_result
                break
        for possible_result in self.possible_results:
            if self.num_unexpected[possible_result] != 0:
                self.result = possible_result
        self.color = possible_result_colors[possible_results.index(self.result)]
        self.expected = self.expected_result == self.result

    def __repr__(self):
        if len(self.results) == 0:
            return f"Empty {self.multiple_tasks.name} result"
        elif len(self.results) == 1:
            return f"Single {self.multiple_tasks.name} result: " + self.results[0].get_description()
        else:
            exclude_result_filter = "|".join(filter(lambda possible_result: (self.possible_result_colors[self.possible_results.index(possible_result)] == COLOR_GREEN or
                                                                             self.possible_result_colors[self.possible_results.index(possible_result)] == COLOR_CYAN),
                                                    self.possible_results))
            details = self.get_details(exclude_result_filter=exclude_result_filter, include_parameters=True)
            return ("" if details.strip() == "" else "\nDetails:\n" + details + "\n\n") + \
                   f"Multiple {self.multiple_tasks.name} results: " + self.color + self.result + COLOR_RESET + ", " + \
                   "summary: " + self.get_summary()

    def is_all_results_done(self):
        return self.num_expected["DONE"] == len(self.results)

    def is_all_results_expected(self):
        for key, value in self.num_unexpected.items():
            if value != 0:
                return False
        return True

    def print_result(self, output_stream=sys.stdout, **kwargs):
        print(self.get_summary(), file=output_stream)
        for task_result in self.results:
            if task_result.color != COLOR_GREEN:
                print("  ", end="", file=output_stream)
                task_result.print_result(output_stream=output_stream, **kwargs)

    def count_results(self, result, expected):
        num = sum(e.result == result and e.expected == expected for e in self.results)
        if num != 0:
            self.num_different_results += 1
        return num

    def get_result_class_texts(self, result, color, num_expected, num_unexpected):
        texts = []
        if num_expected != 0:
            texts.append(color + str(num_expected) + " " + result + (COLOR_GREEN + " (expected)" + COLOR_RESET if color != COLOR_GREEN else "") + COLOR_RESET)
        if num_unexpected != 0:
            texts.append(color + str(num_unexpected) + " " + result + (COLOR_YELLOW + " (unexpected)" + COLOR_RESET if color != COLOR_GREEN else "") + COLOR_RESET)
        return texts

    def get_description(self, **kwargs):
        return f"Multiple {self.multiple_tasks.name}s: " + self.get_summary()

    def get_summary(self):
        if len(self.results) == 1:
            return self.results[0].get_description()
        else:
            texts = []
            if self.num_different_results != 1:
                texts.append(str(len(self.results)) + " TOTAL")
            for possible_result in self.possible_results:
                texts += self.get_result_class_texts(possible_result, self.possible_result_colors[self.possible_results.index(possible_result)], self.num_expected[possible_result], self.num_unexpected[possible_result])
            return ", ".join(texts) + (" in " + str(datetime.timedelta(seconds=self.elapsed_wall_time)) if self.elapsed_wall_time else "")

    def get_details(self, separator="\n  ", result_filter=None, exclude_result_filter=None, **kwargs):
        texts = []
        def matches_possible_result(task_result, possible_result):
            return task_result and task_result.result == possible_result and \
                   matches_filter(task_result.result, result_filter, exclude_result_filter, True)
        for possible_result in self.possible_results:
            for result in filter(lambda result: matches_possible_result(result, possible_result), self.results):
                texts.append(result.get_description(**kwargs))
        return "  " + separator.join(texts)

    def get_done_results(self, exclude_expected=True):
        return self.filter_results(result_filter="DONE", exclude_expected_result_filter="ERROR" if exclude_expected else None)

    def get_cancel_results(self, exclude_expected=True):
        return self.filter_results(result_filter="CANCEL", exclude_expected_result_filter="ERROR" if exclude_expected else None)

    def get_error_results(self, exclude_expected=True):
        return self.filter_results(result_filter="ERROR", exclude_expected_result_filter="ERROR" if exclude_expected else None)

    def get_unexpected_results(self):
        return self.filter_results(exclude_result_filter="SKIP|CANCEL", exclude_expected_test_result=True)

    def filter_results(self, result_filter=None, exclude_result_filter=None, expected_result_filter=None, exclude_expected_result_filter=None, exclude_expected_test_result=False, exclude_error_message_filter=None, error_message_filter=None, full_match=True):
        def matches_test_result(test_result):
            return (not exclude_expected_test_result or test_result.expected_result != test_result.result) and \
                   matches_filter(test_result.result, result_filter, exclude_result_filter, full_match) and \
                   matches_filter(test_result.expected_result, expected_result_filter, exclude_expected_result_filter, full_match) and \
                   matches_filter(test_result.error_message, error_message_filter, exclude_error_message_filter, full_match)
        filtered_results = list(filter(matches_test_result, self.results))
        filtered_tasks = list(map(lambda result: result.task, filtered_results))
        multiple_tasks = self.multiple_tasks.recreate(tasks=filtered_tasks, concurrent=self.multiple_tasks.concurrent)
        return self.recreate(multiple_tasks=multiple_tasks, results=filtered_results)

    def recreate(self, **kwargs):
        return self.__class__(**dict(dict(self.locals, **self.kwargs), **kwargs))

    def rerun(self, **kwargs):
        return self.multiple_tasks.rerun(**kwargs)

class Task:
    def __init__(self, name="task", task_result_class=TaskResult, **kwargs):
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.name = name
        self.task_result_class = task_result_class
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def set_cancel(self, cancel):
        self.cancel = cancel

    def get_parameters_string(self, **kwargs):
        return self.name

    def get_progress_string(self, index, count):
        count_str = str(count)
        index_str = str(index + 1)
        index_str_padding = "0" * (len(count_str) - len(index_str))
        return "[" + index_str_padding + index_str + "/" + count_str + "] " if index is not None and count is not None else ""

    def run(self, index=None, count=None, print_end=" ", dry_run=False, output_stream=sys.stdout, keyboard_interrupt_handler=None, handle_exception=True, **kwargs):
        if self.cancel:
            return self.task_result_class(task=self, result="CANCEL", reason="Cancel by user")
        else:
            try:
                print((self.get_progress_string(index, count) if index is not None and count is not None else "") + self.get_parameters_string(**kwargs), end=print_end, file=output_stream)
                output_stream.flush()
                with EnabledKeyboardInterrupts(keyboard_interrupt_handler):
                    if not dry_run:
                        start_time = time.time()
                        task_result = self.run_protected(**kwargs)
                        end_time = time.time()
                        task_result.elapsed_wall_time = end_time - start_time
                    else:
                        task_result = self.task_result_class(task=self, result="DONE", reason="Dry run")
            except KeyboardInterrupt:
                task_result = self.task_result_class(task=self, result="CANCEL", reason="Cancel by user")
            except Exception as e:
                if handle_exception:
                    task_result = self.task_result_class(task=self, result="ERROR", reason="Exception during task execution", error_message=e.__repr__(), exception=e)
                else:
                    raise e
            task_result.print_result(complete_error_message=False, output_stream=output_stream)
            return task_result

    def run_protected(self, **kwargs):
        return self.task_result_class(task=self, result="DONE", reason="Task completed")

    def recreate(self, **kwargs):
        return self.__class__(**dict(dict(self.locals, **self.kwargs), **kwargs))

    def rerun(self, **kwargs):
        return self.recreate(**kwargs).run()

class SuccessfulTask(Task):
    def run_protected(self, **kwargs):
        return TaskResult(result="DONE")

class FailingTask(Task):
    def run_protected(self, **kwargs):
        return TaskResult(result="FAIL", possible_results=["DONE", "FAIL"], possible_result_colors=[COLOR_GREEN, COLOR_YELLOW])

class ErroneousTask(Task):
    def run_protected(self, **kwargs):
        1/0

class MultipleTasks:
    def __init__(self, tasks=[], name="task", start=None, end=None, concurrent=True, randomize=False, chunksize=1, pool_class=multiprocessing.pool.ThreadPool, multiple_task_results_class=MultipleTaskResults, **kwargs):
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.tasks = tasks
        self.name = name
        self.start = start
        self.end = end
        self.concurrent = concurrent
        self.randomize = randomize
        self.chunksize = chunksize
        self.pool_class = pool_class
        self.multiple_task_results_class = multiple_task_results_class
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def set_cancel(self, cancel):
        self.cancel = cancel
        for task in self.tasks:
            task.set_cancel(cancel)

    def run(self, **kwargs):
        description = "concurrently" if self.concurrent else "sequentially"
        logger.info(f"Running multiple {self.name}s {description} started")
        if self.cancel:
            task_results = list(map(lambda task: task.task_result_class(task=task, result="CANCEL", reason="Cancel by user"), self.tasks))
            multiple_task_results = self.multiple_task_results_class(multiple_tasks=self, results=task_results)
        else:
            try:
                start_time = time.time()
                multiple_task_results = self.run_protected(**kwargs)
                end_time = time.time()
                multiple_task_results.elapsed_wall_time = end_time - start_time
            except KeyboardInterrupt:
                task_results = list(map(lambda task: task.task_result_class(task=task, result="CANCEL", reason="Cancel by user"), self.tasks))
                multiple_task_results = self.multiple_task_results_class(multiple_tasks=self, results=task_results)
        logger.info(f"Running multiple {self.name}s {description} ended")
        return multiple_task_results

    def run_protected(self, **kwargs):
        tasks = self.tasks[self.start:self.end+1] if self.start is not None and self.end is not None else self.tasks
        task_count = len(tasks)
        for task in tasks:
            task.set_cancel(False)
        if self.randomize:
            tasks = random.sample(tasks, k=len(tasks))
        if self.concurrent:
            try:
                pool = self.pool_class(multiprocessing.cpu_count())
                partially_applied_function = functools.partial(run_task_with_capturing_output, tasks=tasks, task_count=task_count, **dict(kwargs, keyboard_interrupt_handler=None))
                map_results = pool.map_async(partially_applied_function, tasks, chunksize=self.chunksize)
                task_results = map_results.get(0xFFFF)
            except KeyboardInterrupt:
                for task in tasks:
                    task.set_cancel(True)
                task_results = map_results.get(0xFFFF)
        else:
            keyboard_interrupt_handler = KeyboardInterruptHandler()
            with DisabledKeyboardInterrupts(keyboard_interrupt_handler):
                cancel = False
                task_results = []
                task_index = 0
                for task in tasks:
                    task.set_cancel(cancel)
                    result = task.run(**dict(kwargs, keyboard_interrupt_handler=keyboard_interrupt_handler, index=task_index, count=task_count))
                    if result.result == "CANCEL":
                        cancel = True
                    task_results.append(result)
                    task_index = task_index + 1
        return self.multiple_task_results_class(multiple_tasks=self, results=task_results)

    def recreate(self, **kwargs):
        return self.__class__(**dict(dict(self.locals, **self.kwargs), **kwargs))

    def rerun(self, **kwargs):
        return self.recreate(**kwargs).run()

def run_task_with_capturing_output(task, tasks=None, task_count=None, output_stream=sys.stdout, **kwargs):
    task_output_stream = io.StringIO()
    task_index = tasks.index(task)
    task_result = task.run(output_stream=task_output_stream, **dict(kwargs, index=task_index, count=task_count))
    print(task_output_stream.getvalue(), end="", file=output_stream)
    return task_result

def run_tasks(tasks, task_result_class=TaskResult, multiple_tasks_class=MultipleTasks, multiple_task_results_class=MultipleTaskResults, **kwargs):
    multiple_tasks = multiple_tasks_class(multiple_tasks=tasks, **kwargs)
    return multiple_tasks.run(**kwargs)

def run_task_tests(multiple_tasks_class=MultipleTasks, task_class=Task, expected_result="DONE"):
    multiple_tasks = multiple_tasks_class([task_class(), task_class()])
    multiple_task_results1 = multiple_tasks.run()
    assert(multiple_task_results1.result == expected_result)
    multiple_task_results2 = multiple_task_results1.get_unexpected_results()
    assert(len(multiple_task_results2.multiple_tasks.tasks) == 0)
    multiple_task_results3 = multiple_task_results1.rerun(concurrent=False)
    assert(multiple_task_results3.result == expected_result)
