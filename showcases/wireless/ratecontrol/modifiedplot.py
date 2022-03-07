import math
from omnetpp.scave import results, chart, utils, ideplot
import matplotlib.pyplot as plt
import pandas as pd

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


def add_to_dataframe(df, style_tuple_list, default_dict={}):
    """
    Adds 'additional_style' column to dataframe. The concent of this column is added to 'style' object when plotting.
    style_tuple_list: [(column, value, {style dictionary}), (...), ...]:
        add style dictionary concents to 'additional_style' column in rows where column=value
    default_dict: {style dictionary}:
        add style to rows not matched by the above
        
    example:
    style_tuple_list = [('legend', 'eth[0]', {'linestyle': '--', 'linewidth': 2}), ('legend', 'eth[1]', {'linestyle': '-', 'linewidth': 2, 'marker': 's', 'markersize': 4})]
    default_dict = {'linestyle': '-', 'linewidth': 1}
    """
    
    df['additional_style'] = None
    
    for i in style_tuple_list:
        column = i[0]
        print("column: ", column)
        value = i[1]
        print("value: ", value)
        style_tuple = i[2]
        print("style_tuple: ", style_tuple)
        
        for i in range(0,len(df)):
            if (df[column][i] == value and df['additional_style'][i] == None):
                df['additional_style'][i] = str(style_tuple)
                
    for i in range(0,len(df)):
        if (df['additional_style'][i] == None):
            print('adding default stuff')
            df['additional_style'][i] = str(default_dict)

    return df