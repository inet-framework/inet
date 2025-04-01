from omnetpp.cppindex import CodeBase

codebase = CodeBase()
codebase.analyze("src")

def has_method(clazz, method_name):
    return any(f.name == method_name for f in clazz.functions)

def has_inherited_method(clazz, method_name):
    for b in clazz.get_all_base_classes():
        if has_method(b, method_name):
            return True
    return False

def get_all_base_class_names(clazz):
    res = set()
    for b in clazz.get_all_base_classes():
        res.update(b.get_base_class_names())
    return res

def is_module(clazz):
    b = get_all_base_class_names(clazz)
    return "cSimpleModule" in b or "omnetpp::cSimpleModule" in b or "cModule" in b or "omnetpp::cModule" in b

def print_clazz_list(clazz_list):
    for clazz in clazz_list:
        print(clazz.name, clazz.get_location().file)    

modules = [clazz for clazz in codebase.get_classes() if is_module(clazz)]
# print("Modules:", modules, len(modules))
# print()

defines_it = [clazz for clazz in modules if has_method(clazz, "resolveDirective")]
print("Defines resolveDirective():")
print_clazz_list(defines_it)
print()

needs_override = [clazz for clazz in modules
                  if not has_method(clazz, "resolveDirective")
                  and has_inherited_method(clazz, "resolveDirective")]
print("Supports resolveDirective() but does not override it:")
print_clazz_list(needs_override)
print()

no_support = [clazz for clazz in modules
              if not has_method(clazz, "resolveDirective")
              and not has_inherited_method(clazz, "resolveDirective")]
print("No resolveDirective() in it or base classes:")
print_clazz_list(no_support)
print() 
