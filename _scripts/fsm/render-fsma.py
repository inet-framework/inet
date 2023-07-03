#
# Rudimentary parsing and visualization of state machines based on the FSMA macros in the source code.
#

import re
import subprocess
import networkx as nx
import matplotlib.pyplot as plt
from collections import namedtuple
from pyparsing import *

Fsm = namedtuple('Fsm', 'name states')
State = namedtuple('State', 'name transitions')
Enter = namedtuple('Enter', 'action')
Transition = namedtuple('Transition', 'name target_state condition action')

def preprocess_cpp_source(source_code):
    preprocessor_cmd = ['cpp', '-P', '-']
    preprocessor = subprocess.Popen(preprocessor_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    preprocessed_code, error = preprocessor.communicate(input=source_code)
    if preprocessor.returncode != 0:
        raise RuntimeError(f'C++ preprocessor failed with error:\n{error}')
    return preprocessed_code

def comment_out_preprocessor_directives(source_code):
    return re.sub(r'^\s*#', '//#', source_code, flags=re.MULTILINE)

def parse_fsm(source_code):
    # Discard C++ code up to the beginning of the FSM
    source_code = source_code[source_code.index("FSMA_Switch("):]

    # In transitions, convert the condition and action parts into string constants, to facilitate parsing.
    # An alternative would be to parse using something like this:
    #
    # blah = Word(printables, excludeChars=',(){}"\'')
    # quoted_string = QuotedString('"')
    # apos_string = QuotedString("'")
    # element = blah | quoted_string | apos_string
    #
    # expr = Forward()
    # paren_expr = Combine(Literal("(") + expr[...] + Literal(")"))
    # brace_expr = Combine(Literal("{") + expr[...] + Literal("}"))
    # expr << (element | paren_expr | brace_expr)
    # expr = Optional(expr[...])
    #
    # But whitespace is lost during parsing. Would be possible to re-obtain it from
    # the original source, but we'd need locations for that.
    # Could use Located class or locatedExpr().

    source_code = preprocess_cpp_source("""
        #define FSMA_Enter(action)  FSMA_Enter(#action)
        #define FSMA_Event_Transition(name,condition,target,action) FSMA_Event_Transition(name, #condition, target, #action)
        """ +
        comment_out_preprocessor_directives(source_code))

    print(source_code)

    # Define the FSM syntax using PyParsing
    name = Word(alphas, alphanums+'_-')
    condition = QuotedString('"', escChar='\\')
    action = QuotedString('"', escChar='\\')
    transition = Group("FSMA_Event_Transition" + "(" + name + "," + condition + "," + name + "," + action + ");")
    enter_block = Group("FSMA_Enter" + "(" + action + ");")
    state_block = Group("FSMA_State" + "(" + name + ")" + "{" + Optional(enter_block) + ZeroOrMore(transition) + "}")
    fsm_block = Group("FSMA_Switch(" + name + ")" + "{" + OneOrMore(state_block) + "}")

    def make_transition(tokens):
        _, event_name, _, condition, _, target_state, _, action, _ = tokens[0]
        return Transition(name=event_name, target_state=target_state, condition=condition, action=action)

    def make_enter(tokens):
        _, action, _ = tokens[0]
        return Enter(action=action)

    def make_state(tokens):
        _, state_name, _, _, *transitions = tokens[0][:-1]
        #TODO remove Enter
        return State(name=state_name, transitions=transitions)

    def make_fsm(tokens):
        _, fsm_name, _, _, *states = tokens[0][:-1]
        return Fsm(name=fsm_name, states=states)

    # Define PyParsing grammar actions
    transition.setParseAction(make_transition)
    enter_block.setParseAction(make_enter)
    state_block.setParseAction(make_state)
    fsm_block.setParseAction(make_fsm)
    result = fsm_block.parseString(source_code)
    return result[0]

def fsm_to_graph(fsm):
    G = nx.DiGraph()
    G.name = fsm.name
    for state in fsm.states:
        G.add_node(state.name)
    for state in fsm.states:
        for transition in [p for p in state.transitions if type(p)==Transition]:
            G.add_edge(state.name, transition.target_state, event=transition.name, condition=transition.condition, action=transition.action)
    return G

def draw_fsm_graph(G):
    pos = nx.spring_layout(G)  # Position the nodes using a spring layout algorithm
    edge_labels = {(u, v): d['event'] for u, v, d in G.edges(data=True)}
    node_labels = {node: node for node in G.nodes}
    nx.draw_networkx(G, pos, with_labels=False, node_color='lightblue', edge_color='gray', node_size=2000)
    nx.draw_networkx_labels(G, pos, labels=node_labels)
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_size=6)

# EXAMPLE:

example_fname = "../../src/inet/linklayer/csmaca/CsmaCaMac.cc"

source_code = open(example_fname).read()
fsm = parse_fsm(source_code)
print(fsm)
G = fsm_to_graph(fsm)
print(G)
draw_fsm_graph(G)
plt.show()


