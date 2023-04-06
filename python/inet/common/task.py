"""
Provides abstractions for generic tasks and their results.

Tasks are primarily useful because they can be created, passed around and stored
before actually being run. This separation allows running the same tasks multiple
times.

For example, creating an empty task and running it:

.. code-block:: python

    t = Task()
    t.run()

Similarly, creating a multiple tasks object containing two empty tasks and running it:

.. code-block:: python

    mt = MultipleTasks([Task(), Task()])
    mt.run()

Please note that undocumented features are not supposed to be used by the user.
"""

import dask
import datetime
import functools
import hashlib
import logging
import multiprocessing
import multiprocessing.pool
import socket
import sys
import time
import traceback

from inet.common.util import *

_logger = logging.getLogger(__name__)

class TaskResult:
    """
    Represents a task result that is produced when a :py:class:`Task` is run. The most important attributes of a task
    result are the result, reason and error_message.
    """

    def __init__(self, task=None, result="DONE", expected_result="DONE", reason=None, error_message=None, exception=None, store_complete_binary_hash=False, store_complete_source_hash=False, store_partial_binary_hash=False, store_partial_source_hash=False, elapsed_wall_time=None, possible_results=["DONE", "SKIP", "CANCEL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_RED], **kwargs):
        """
        Initializes a new task result object.

        Parameters:
            task (:py:class:`Task`):
                The original task that was run when this task result was produced.

            result (string):
                The result of the task execution. The value must be one of the values in the possible_results attribute.

            expected_result (string):
                The originally expected result of the task execution. The value must be one of the values in the possible_results attribute.

            reason (string or None):
                An optional human readable explanation for the task result.

            error_message (string or None):
                An optional error message that is most often extracted from an exception raised during the task execution.

            exception (Exception):
                An optional exception that was raised during the task execution.

            store_complete_binary_hash (bool):
                Requests storing the hash of the complete binary distribution that is needed to run the original task.

            store_complete_source_hash (bool):
                Requests storing the hash of the complete source distribution that is needed to run the original task.

            store_partial_binary_hash (bool):
                Requests storing the hash of the optimized partial binary distribution that is needed to run the original task.

            store_partial_source_hash (bool):
                Requests storing the hash of the optimized partial source distribution that is needed to run the original task.

            elapsed_wall_time (number):
                The elapsed time while the task was running.

            possible_results (List of string):
                The list of possible results with which the execution of the original task can end.

            possible_result_colors (List of string):
                The list of colors for the corresponding possible results.
        """
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.task = task
        self.hostname = socket.gethostname()
        self.run_at = time.time()
        self.result = result
        self.expected_result = expected_result
        self.expected = expected_result == result
        self.reason = reason
        self.error_message = error_message
        self.exception = exception
        self.elapsed_wall_time = elapsed_wall_time
        self.possible_results = possible_results
        self.possible_result_colors = possible_result_colors
        self.color = possible_result_colors[possible_results.index(result)]
        self.complete_binary_hash = hex_or_none(task.get_hash(complete=True, binary=True)) if task and store_complete_binary_hash else None
        self.complete_source_hash = hex_or_none(task.get_hash(complete=True, binary=False)) if task and store_complete_source_hash else None
        self.partial_binary_hash = hex_or_none(task.get_hash(complete=False, binary=True)) if task and store_partial_binary_hash else None
        self.partial_source_hash = hex_or_none(task.get_hash(complete=False, binary=False)) if task and store_partial_source_hash else None

    def __repr__(self):
        return "Result: " + self.get_description()

    def get_hash(self, **kwargs):
        return self.task.get_hash(**kwargs)

    def get_description(self, complete_error_message=True, include_parameters=False, **kwargs):
        return (self.task.get_parameters_string() + " " if include_parameters else "") + \
                self.color + self.result + COLOR_RESET + \
                ((COLOR_YELLOW + " (unexpected)" + COLOR_RESET) if not self.expected and self.color != COLOR_GREEN else "") + \
                ((COLOR_GREEN + " (expected)" + COLOR_RESET) if self.expected and self.color != COLOR_GREEN else "") + \
               (" (" + self.reason + ")" if self.reason else "") + \
               (" " + self.get_error_message(complete_error_message=complete_error_message) if self.result == "ERROR" else "")

    def get_error_message(self, **kwargs):
        return self.error_message or "<No error message>"

    def print_result(self, complete_error_message=False, output_stream=sys.stdout, **kwargs):
        print(self.get_description(complete_error_message=complete_error_message), file=output_stream)

    def recreate(self, **kwargs):
        return self.__class__(**dict(dict(self.locals, **self.kwargs), **kwargs))

    def rerun(self, **kwargs):
        """
        Runs the original task again that created this task result.

        Returns (:py:class:`TaskResult`):
            The task result.
        """
        return self.task.rerun(**kwargs)

class MultipleTaskResults:
    """
    Represents multiple task results that are created when :py:class:`MultipleTasks` are run.
    """

    def __init__(self, multiple_tasks=None, results=[], expected_result="DONE", elapsed_wall_time=None, possible_results=["DONE", "SKIP", "CANCEL", "ERROR"], possible_result_colors=[COLOR_GREEN, COLOR_CYAN, COLOR_CYAN, COLOR_RED], **kwargs):
        """
        Initializes a new multiple task results object.

        Parameters:
            multiple_tasks (:py:class:`MultipleTasks`):
                The original multiple tasks object that was run when this multiple task result was produced.

            results (List of :py:class:`TaskResult`):
                The list of individual task results corresponding to the individual tasks.

            expected_result="DONE" (string):
                The expected result of the multiple task execution. The value must be one of the values in the possible_results attribute.

            elapsed_wall_time (number):
                The elapsed time while the task was running.

            possible_results (List of string):
                The list of possible results with which the execution of the original multiple task can end.

            possible_result_colors (List of string):
                The list of colors for the corresponding possible results.
        """
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

    def get_skip_results(self, exclude_expected=True):
        return self.filter_results(result_filter="SKIP", exclude_expected_result_filter="ERROR" if exclude_expected else None)

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
        """
        Runs the original multiple tasks again that created this multiple task results.

        Returns (:py:class:`MultipleTaskResults`):
            The multiple task results.
        """
        return self.multiple_tasks.rerun(**kwargs)

class Task:
    """
    Represents a self-contained operation that captures all necessary information in order to be run.
    """

    def __init__(self, name="task", action="", print_run_start_separately=True, task_result_class=TaskResult, **kwargs):
        """
        Initializes a new task object.

        Parameters:
            name (string):
                A human readable short description of the task, usually a noun.

            action (string):
                A human readable short description of the operation the task is carrying out, usually a verb.

            print_run_start_separately (bool):
                Specifies if the start and end of the task's execution is printed separately wrapping around the execution.

            task_result_class (string):
                The Python class name of the produced task result object.
        """
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.name = name
        self.action = action
        self.print_run_start_separately = print_run_start_separately
        self.task_result_class = task_result_class
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def get_hash(self, **kwargs):
        hasher = hashlib.sha256()
        hasher.update(self.name.encode("utf-8"))
        return hasher.digest()

    def set_cancel(self, cancel):
        self.cancel = cancel

    def get_progress_string(self, index, count):
        count_str = str(count)
        index_str = str(index + 1)
        index_str_padding = "0" * (len(count_str) - len(index_str))
        return "[" + index_str_padding + index_str + "/" + count_str + "]" if index is not None and count is not None else ""

    def get_action_string(self, **kwargs):
        return self.action

    def get_parameters_string(self, **kwargs):
        return ""

    def print_run_start(self, index=None, count=None, print_end=" ", output_stream=sys.stdout, **kwargs):
        progress_string = (self.get_progress_string(index, count) if index is not None and count is not None else "")
        action_string = self.get_action_string(**kwargs)
        parameters_string = self.get_parameters_string(**kwargs)
        elements = [e for e in [progress_string, action_string, parameters_string] if e != ""]
        print(" ".join(elements), end=print_end, file=output_stream)
        output_stream.flush()

    def print_run_end(self, task_result, output_stream=sys.stdout, **kwargs):
        task_result.print_result(complete_error_message=False, output_stream=output_stream)

    def run(self, dry_run=False, keyboard_interrupt_handler=None, handle_exception=True, **kwargs):
        """
        Runs the task.

        Parameters:
            dry_run (bool):
                Specifies to skip the actual running of the task but do everything else.

            keyboard_interrupt_handler (:py:class:`omnetpp.common.KeyboardInterruptHandler` or None):
                Provides a class that will handle keyboard interrupts. This allows seamless exit from running multiple
                tasks.

            handle_exception (bool):
                Specifies if exceptions are caught and processed or passed to the caller.

        Returns (:py:class:`TaskResult`):
            The task result.
        """
        if self.cancel:
            return self.task_result_class(task=self, result="CANCEL", reason="Cancel by user")
        else:
            try:
                if self.print_run_start_separately:
                    self.print_run_start(**kwargs)
                with EnabledKeyboardInterrupts(keyboard_interrupt_handler):
                    if dry_run:
                        task_result = self.task_result_class(task=self, result="DONE", reason="Dry run")
                    else:
                        start_time = time.time()
                        task_result = self.run_protected(**kwargs)
                        end_time = time.time()
                        task_result.elapsed_wall_time = end_time - start_time
            except KeyboardInterrupt:
                task_result = self.task_result_class(task=self, result="CANCEL", reason="Cancel by user")
            except Exception as e:
                if handle_exception:
                    task_result = self.task_result_class(task=self, result="ERROR", reason="Exception during task execution", error_message=e.__repr__(), exception=e)
                else:
                    raise e
            if not self.print_run_start_separately:
                self.print_run_start(**kwargs)
            self.print_run_end(task_result, **kwargs)
            return task_result

    def run_protected(self, **kwargs):
        """
        Runs the task in the protected environment wrapped by :py:meth:`run`. This method is expected to be overridden
        by derived classes. The default implementation simply returns a task result with "DONE" result code.

        Parameters:
            kwargs (dict):
                Not used in this implementation.

        Returns (:py:class:`TaskResult`):
            The task result.
        """
        return self.task_result_class(task=self, result="DONE", reason="Task completed")

    def recreate(self, **kwargs):
        return self.__class__(**dict(dict(self.locals, **self.kwargs), **kwargs))

    def rerun(self, **kwargs):
        """
        Runs the task again.

        Returns (:py:class:`TaskResult`):
            The task result.
        """
        if len(kwargs) == 0:
            return self.run()
        else:
            return self.recreate(**kwargs).run()

def _run_task(task):
    return task.run()

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
    """
    Represents multiple tasks that can be run together. 
    """

    def __init__(self, tasks=[], name="task", concurrent=True, randomize=False, chunksize=1, scheduler="thread", cluster=None, multiple_task_results_class=MultipleTaskResults, **kwargs):
        """
        Initializes a new multiple tasks object.

        Parameters:
            tasks (List of :py:class:`Task`):
                The list of individual tasks that are run when this multiple tasks is run.

            name (string):
                A human readable short description of the multiple tasks, usually a noun.

            concurrent (bool):
                Specifies if the individual tasks are run sequentially or concurrently.

            randomize (bool):
                Specifies if the order of execution is random or follows the order of storage for the tasks.

            chunksize (integer):
                The number of tasks that are run together in a single batch if the tasks are running concurrently.

            scheduler (string):
                Specifies how the tasks are scheduled. Valid values are "process", "thread", and "cluster".

            multiple_task_results_class (string):
                The Python class name of the produced multiple task results object.
        """
        self.locals = locals()
        self.locals.pop("self")
        self.kwargs = kwargs
        self.tasks = tasks
        self.name = name
        self.concurrent = concurrent
        self.randomize = randomize
        self.chunksize = chunksize
        self.cluster = cluster
        self.scheduler = scheduler
        self.multiple_task_results_class = multiple_task_results_class
        self.cancel = False

    def __repr__(self):
        return repr(self)

    def set_cancel(self, cancel):
        self.cancel = cancel
        for task in self.tasks:
            task.set_cancel(cancel)

    def get_description(self):
        concurrency_description = "concurrently" if self.concurrent else "sequentially"
        return f"{self.name}s {concurrency_description}"

    def run(self, **kwargs):
        """
        Runs all tasks sequentially or concurrently.

        Parameters:
            kwargs (dict): Additional  parameters are inherited from :py:meth:`Task.run`.

        Returns (:py:class:`MultipleTaskResults`):
            The task results.
        """
        _logger.info(f"Running multiple {self.get_description()} started")
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
        _logger.info(f"Running multiple {self.get_description()} ended")
        return multiple_task_results

    def run_protected(self, **kwargs):
        if self.scheduler == "cluster":
            delayed_results = list(map(lambda task: dask.delayed(_run_task)(task), self.tasks))
            return self.multiple_task_results_class(multiple_tasks=self, results=dask.compute(*delayed_results))
        else:
            tasks = self.tasks
            task_count = len(tasks)
            for task in tasks:
                task.set_cancel(False)
            if self.randomize:
                tasks = random.sample(tasks, k=len(tasks))
            if self.concurrent:
                pool_class = multiprocessing.pool.ThreadPool if self.scheduler == "thread" else (multiprocessing.Pool if self.scheduler == "process" else None)
                try:
                    pool = pool_class(multiprocessing.cpu_count())
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
        """
        Runs the tasks again.

        Returns (:py:class:`MultipleTaskResults`):
            The task results.
        """
        if len(kwargs) == 0:
            return self.run()
        else:
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
