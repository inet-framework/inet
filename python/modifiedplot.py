import math
from omnetpp.scave import results, chart, utils, ideplot
import matplotlib.pyplot as plt
import pandas as pd
import ast
import re
from matplotlib.lines import Line2D

# debug = False

default_colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']

def plot_vectors(df, props, legend_func=utils.make_legend_label):
    """
    Creates a line plot from the dataframe, with styling and additional input
    coming from the properties. Each row in the dataframe defines a series.

    Colors and markers are assigned automatically. The `cycle_seed` property
    allows you to select other combinations if the default one is not suitable.

    A function to produce the legend labels can be passed in. By default,
    `make_legend_label()` is used, which offers many ways to influence the
    legend via datataframe columns and chart properties. In the absence of
    more specified settings, the legend is normally computed from columns which best
    differentiate among the vectors.

    Parameters:

    - `df`: the dataframe
    - `props` (dict): the properties
    - `legend_func`: the function to produce custom legend labels

    Columns of the dataframe:

    - `vectime`, `vecvalue` (Numpy `ndarray`'s of matching sizes): the x and y coordinates for the plot
    - `interpolationmode` (str, optional): this column normally comes from a result attribute, and determines how the points will be connected
    - `legend` (optional): legend label for the series; if missing, legend labels are derived from other columns
    - `name`, `title`, `module`, etc. (optional): provide input for the legend

    Notable properties that affect the plot:

    - `title`: plot title (autocomputed if missing)
    - `legend_labels`: selects whether to prefer the `name` or the `title` column for the legend
    - `drawstyle`: Matplotlib draw style; if present, it overrides the draw style derived from `interpolationmode`.
    - `linestyle`, `linecolor`, `linewidth`, `marker`, `markersize`: styling
    - `cycle_seed`: Alters the sequence in which colors and markers are assigned to series.
    """
    p = ideplot if chart.is_native_chart() else plt

    def get_prop(k):
        return props[k] if k in props else None

    title_cols, legend_cols = utils.extract_label_columns(df, props)
    
    if 'order' in df.columns:
        df.sort_values(by='order', inplace=True)
    else:
        df.sort_values(by=legend_cols, inplace=True)
    for t in df.itertuples(index=False):
        style = utils._make_line_args(props, t, df)
#        if t.propertyname != '':
#            style[t.propertyname] = t.propertyvalue
        style_dict = eval(t.additional_style)
        # print("style_dict:", style_dict)
        for i in style_dict.items():
            style[i[0]] = i[1]
        # print("style:", style)
        p.plot(t.vectime, t.vecvalue, label=legend_func(legend_cols, t, props), **style)

    title = get_prop("title") or utils.make_chart_title(df, title_cols)
    utils.set_plot_title(title)

    p.ylabel(utils.make_chart_title(df, ["title"]))
    
def plot_vectors_separate(df, props, legend_func=utils.make_legend_label, debug=False):
    """
    Modified version of the built-in plot_vectors_separate() function, with the additional functionality
    of ordering the dataframe before plotting, based on the 'order' column. The order of the line colors and
    the legend order can be controlled this way. It also returns the list of axes used in the plot for further processing.
    
    Also adds the content of the 'additional_style' column to the style object before plotting.
    
        Original description:
        
        This is very similar to `plot_vectors`, with identical usage.
        The only difference is in the end result, where each vector will
        be plotted in its own separate set of axes (coordinate system),
        arranged vertically, with a shared X axis during navigation.
    """
    def get_prop(k):
        return props[k] if k in props else None
    
    ax_list = []

    title_cols, legend_cols = utils.extract_label_columns(df, props)

    if 'order' in df.columns:
        if debug: print("\nsorting by order\n")
        df.sort_values(by=['order'], inplace=True)

    ax = None
    for i, t in enumerate(df.itertuples(index=False)):
        style = utils._make_line_args(props, t, df)
        if 'additional_style' in df.columns:
            style_dict = eval(t.additional_style)
            for j in style_dict.items():
                style[j[0]] = j[1]
        ax = plt.subplot(df.shape[0], 1, i+1, sharex=ax)

        if i != df.shape[0]-1:
            plt.setp(ax.get_xticklabels(), visible=False)
            ax.xaxis.get_label().set_visible(False)

        plt.plot(t.vectime, t.vecvalue, label=legend_func(legend_cols, t, props), **style)
        ax_list.append(ax)

    plt.subplot(df.shape[0], 1, 1)

    title = get_prop("title") or make_chart_title(df, title_cols)
    utils.set_plot_title(title)
    
    return ax_list
    
def plot_vectors_separate_grouped(df_list, props, legend_func=utils.make_legend_label):
    """
    This is a modified version of the built-in plot_vectors_separate() function. It takes a list of dataframes,
    and plots each dataframe on its own subplot. Useful for plotting multiple lines on a subplot, as the built-in
    function can't do that currently.
    
    The dataframes can also contain 'additional_style' columns.
    """
    p = ideplot if chart.is_native_chart() else plt
    
    title = ""
    ax_list = []
    for j in range(0, len(df_list)):
        df = df_list[j]

        def get_prop(k):
            return props[k] if k in props else None
    
        title_cols, legend_cols = utils.extract_label_columns(df, props)
        
        ax = None
        ax = plt.subplot(len(df_list), 1, j+1, sharex=ax)
    
        if 'order' in df.columns:
            df.sort_values(by='order', inplace=True)
        else:
            df.sort_values(by=legend_cols, inplace=True)
            
        for t in df.itertuples(index=False):
            style = utils._make_line_args(props, t, df)
            if 'additional_style' in df.columns:
                style_dict = eval(t.additional_style)
                for i in style_dict.items():
                    style[i[0]] = i[1]
            p.plot(t.vectime, t.vecvalue, label=legend_func(legend_cols, t, props), **style)
        
        if j == 0:
            title = get_prop("title") or utils.make_chart_title(df, title_cols)

            #utils.set_plot_title(title)
            plt.suptitle(title)
        ax_list.append(ax)

    p.ylabel(utils.make_chart_title(df, ["title"]))
    
    return ax_list

def add_to_dataframe(df, style_tuple_list=[], default_dict={}, order={}, debug=False):
    """
    Adds 'additional_style' column to dataframe. The concent of this column is added to 'style' object when plotting.
    Can also specify row order in dataframe (e.g. for ordering items in legend).
    style_tuple_list: [(column, value, {style dictionary}), (...), ...]:
        add style dictionary concents to 'additional_style' column in rows where column=value
    default_dict: {style dictionary}:
        add style to rows not matched by the above
    order: {'configname': order, ...}
        2-member list: first one is the column in the dataframe to order by, second is a dict with the order numbers
        
    example:
    style_tuple_list = [('legend', 'eth[0]', {'linestyle': '--', 'linewidth': 2}), ('legend', 'eth[1]', {'linestyle': '-', 'linewidth': 2, 'marker': 's', 'markersize': 4})]
    default_dict = {'linestyle': '-', 'linewidth': 1}
    order = ['configname', {'Default_config': 1, 'Advanced_config': 0, 'Manual_config': 2}]
    
    Note that the value parameter in style_tuple_list and the order can contain regex (e.g. .*foo). Make sure to escape regex characters such as [ and ] with \
    """
    
    df['additional_style'] = None
    
    def add_separator():
        print("")
    if style_tuple_list:
        for i in style_tuple_list:
            column = i[0]
            value = i[1]
            style_tuple = i[2]
            
            for i in range(0,len(df)):
                pattern = re.compile(value)
                match = re.fullmatch(pattern, df[column][i])
                if match != None:
                    if debug:
                        print("MATCH FOUND:")
                        print("    column:", column)
                        print("    value:", value)
                        print("    match:", match.group())
                    if (df['additional_style'][i] == None):
                        df['additional_style'][i] = str(style_tuple)
                        if debug: print("Adding style tuple:", str(style_tuple))
                        if debug: add_separator()
                    else:
                        orig_style_dict = ast.literal_eval(df['additional_style'][i])       # convert already added style to dict
                        if debug: print("Adding style tuple to existing style dict", orig_style_dict, type(orig_style_dict))
                        orig_style_dict.update(style_tuple)                                 # add new style to dict
                        if debug: print("New style dict", orig_style_dict, type(orig_style_dict))
                        if debug: add_separator()
                        df['additional_style'][i] = str(orig_style_dict)                    # add to dataframe as string
                    
        for i in range(0,len(df)):
            if (df['additional_style'][i] == None):
                if debug: print('adding default stuff')
                df['additional_style'][i] = str(default_dict)
            
    if order:
        # order the dataframe
        df['order'] = None
        
        order_column = order[0]
        order_dict = order[1]
        if debug: print('order_column:', order_column, '\norder_dict', order_dict)
        for i in order_dict.items():
            for j in range(0,len(df)):
                # if df[order_column][j] == i[0]:
                pattern = re.compile(i[0])
                match = re.fullmatch(pattern, df[order_column][j])
                if match != None:
                    df['order'][j] = i[1]
                    
        if debug: 
            print("order added:")
            print(df['order'], df)

    return df

def multidimensional_legend(style_tuple_list, labels = [], handles = [], debug=False):
    """
    Can create multi-dimensional legend, where one aspect of a line (e.g. color) represents a dimension, another aspect (e.g. linestyle of solid, dashed or dotted) represents another dimension,
    as opposed to the default behavior, in which lines in the legend represent the lines on the chart directly. **TODO** not sure this explanation is needed
    
    If existing legend handles and labels are supplied arguments, the new legend entries are appended.
    
    style_tuple_list: [('legend label 1', {dict contaning the style arguments for the legend}), ('legend label 2, {...}), ...]
    
    for example:
    style_tuple_list = [('Label1', {'color': 'orange', 'linestyle': '--', 'markersize': 3}),
                        ('Label2', {'color': 'red', 'markersize': 3})]
                        
    returns legend handles and labels that can be supplied to plt.legend()
    """

    if debug: print("style_tuple_list", style_tuple_list)
    for i in style_tuple_list:
        if debug: print("i", i)
        labels.append(i[0])
        if debug: print("i[1]", i[1])
        handles.append(Line2D([0], [0], **i[1]))
    if debug:
        print("labels", labels)
        print("handles", handles)
    return handles, labels
    