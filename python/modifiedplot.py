import math
from omnetpp.scave import results, chart, utils, ideplot
import matplotlib.pyplot as plt
import pandas as pd
import ast
import re
from matplotlib.lines import Line2D
import numpy as np
import colorsys
import matplotlib as mpl
import inspect
import math
from omnetpp.scave.utils import make_legend_label

def scale_lightness(rgb, scale_l):
    # convert rgb to hls
    h, l, s = colorsys.rgb_to_hls(*rgb)
    # manipulate h, l, s values and return as rgb
    return colorsys.hls_to_rgb(h, min(1, l * scale_l), s = s)

def hextriplet(colortuple):
    return '#' + ''.join(f'{i:02X}' for i in colortuple)

def rgb2hex(r,g,b):
    return "#{:02x}{:02x}{:02x}".format(r,g,b)

def fade_color(color, fakealpha):
     c = mpl.colors.to_rgb(color)
     b = mpl.colors.to_rgb(mpl.rcParams["axes.facecolor"])
     def lerp(a, b, t):
         return a + (b-a)*t
     return (lerp(b[0], c[0], fakealpha), lerp(b[1], c[1], fakealpha), lerp(b[2], c[2], fakealpha))


# debug = False

default_colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']

def get_default_colors():
    default_colors = plt.rcParams['axes.prop_cycle'].by_key()['color']
    print(default_colors)
    return default_colors

def add_global_style_if_needed(global_style, style, debug):
    if global_style != None:
        if debug: print('adding global style: ', global_style)
        style.update(global_style)
        if debug: print('added. "style" dict: ', style)

def plot_vectors(df, props, legend_func=utils.make_legend_label, global_style=None, debug=False):
    """
    Modified version of the built-in plot_vectors() function, with the additional functionality
    of ordering the dataframe before plotting, based on the 'order' column. The order of the line colors and
    the legend order can be controlled this way.
    
    Also adds the content of the 'additional_style' column to the style object before plotting.
    
    Original description:
    
        Creates a line plot from the dataframe, with styling and additional input
        coming from the properties. Each row in the dataframe defines a series.
    
        Colors and markers are assigned automatically. The `cycle_seed` property
        allows you to select other combinations if the default one is not suitable.
    
        A function to produce the legend labels can be passed in. By default,
        `make_legend_label()` is used, which offers many ways to influence the
        legend via dataframe columns and chart properties. In the absence of
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
    
    props['plot_function'] = inspect.currentframe().f_code.co_name

    def get_prop(k):
        return props[k] if k in props else None

    title_cols, legend_cols = utils.extract_label_columns(df, props)
    
    if 'order' in df.columns:
        df.sort_values(by='order', inplace=True)
    else:
        df.sort_values(by=legend_cols, inplace=True)
    for t in df.itertuples(index=False):
        style = utils._make_line_args(props, t, df)
        add_global_style_if_needed(global_style, style, debug)
#        if t.propertyname != '':
#            style[t.propertyname] = t.propertyvalue
        if 'additional_style' in df.columns and t.additional_style != None:
            style_dict = eval(t.additional_style)
            # print("style_dict:", style_dict)
            for i in style_dict.items():
                style[i[0]] = i[1]
        # print("style:", style)
        p.plot(t.vectime, t.vecvalue, label=legend_func(legend_cols, t, props), **style)

    title = get_prop("title") or utils.make_chart_title(df, title_cols)
    utils.set_plot_title(title)

    p.ylabel(utils.make_chart_title(df, ["title"]))
    
def plot_vectors_separate(df, props, legend_func=utils.make_legend_label, global_style=None, share_axes=True, debug=False):
    """
    Modified version of the built-in plot_vectors_separate() function, with the additional functionality
    of ordering the dataframe before plotting, based on the 'order' column. The order of the line colors and
    the legend order can be controlled this way. By default, the x axes of the subplots are shared (so they zoom together); turn this off with 'share_axes=False'. 
    It also returns the list of axes used in the plot for further processing.
    
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
    
    props['plot_function'] = inspect.currentframe().f_code.co_name

    title_cols, legend_cols = utils.extract_label_columns(df, props)

    if 'order' in df.columns:
        if debug: print("\nsorting by order\n")
        df.sort_values(by=['order'], inplace=True)

    ax = None
    for i, t in enumerate(df.itertuples(index=False)):
        style = utils._make_line_args(props, t, df)
        add_global_style_if_needed(global_style, style, debug)
        if 'additional_style' in df.columns and t.additional_style != None:
            style_dict = eval(t.additional_style)
            for j in style_dict.items():
                style[j[0]] = j[1]
        if share_axes:
            share_ax_dict = {'sharex': ax}
        else:
            share_ax_dict = {}
        if debug: print("share_axes:", share_axes, "share_ax_dict:", share_ax_dict)
        ax = plt.subplot(df.shape[0], 1, i+1, **share_ax_dict)

        if i != df.shape[0]-1:
            plt.setp(ax.get_xticklabels(), visible=False)
            ax.xaxis.get_label().set_visible(False)

        plt.plot(t.vectime, t.vecvalue, label=legend_func(legend_cols, t, props), **style)
        ax_list.append(ax)

    plt.subplot(df.shape[0], 1, 1)

    title = get_prop("title") or utils.make_chart_title(df, title_cols)
    utils.set_plot_title(title)
    
    return ax_list
    
def plot_vectors_separate_grouped(df_list, props, legend_func=utils.make_legend_label, layout='vertical', columns=0, rows=0, share_axes='auto', global_style=None, debug=False):
    """
    This is a modified version of the built-in plot_vectors_separate() function. It takes a list of dataframes,
    and plots each dataframe on its own subplot. Useful for plotting multiple lines on a subplot, as the built-in
    function can't do that currently.
    
    The dataframes can also contain 'additional_style' columns.
    
    Layout can be:
    - 'vertical' (default)
    - 'horizontal'
    - 'grid': need to specify number of columns and rows
    
    By default, in the horizontal and vertical layouts, the common axes are shared, so they zoom together.
    Turn this off with 'share_axes'.
    
    share_axes: only for the horizontal and vertical layouts; no axes are shared in the grid layout
    - 'auto' (default): the common axes are shared (x for vertical, y for horizontal)
    - 'none': no shared axes
    ' 'both': both axes are shared
    """
    p = ideplot if chart.is_native_chart() else plt
    
    title = ""
    ax_list = []
    
    props['plot_function'] = inspect.currentframe().f_code.co_name
    
    for j in range(0, len(df_list)):
        df = df_list[j]

        def get_prop(k):
            return props[k] if k in props else None
    
        title_cols, legend_cols = utils.extract_label_columns(df, props)
        
        cols = 0
        rws = 0
        
        shareaxis = None
        
        ax = None
        
        if layout == 'vertical':
            cols = 1
            rws = len(df_list)
            # shareaxis = 'shareax = ax'
            if len(ax_list) != 0:
                if share_axes == 'auto':
                    shareaxis = {'sharex': ax_list[-1]}
                elif share_axes == 'none':
                    shareaxis = {}
                elif share_axes == 'both':
                    shareaxis = {'sharex': ax_list[-1], 'sharey': ax_list[-1]}
                else:
                    assert False, "Wrong value for share_axes: " + str(share_axes) + ". Possible values: 'auto', 'none', 'both'"                
            else:
                shareaxis = {}
            ax = plt.subplot(len(df_list), 1, j+1, **shareaxis)
        elif layout == 'horizontal':
            cols = len(df_list)
            rws = 1
            if len(ax_list) != 0:
                if share_axes == 'auto':
                    shareaxis = {'sharey': ax_list[-1]}
                elif share_axes == 'none':
                    shareaxis = {}
                elif share_axes == 'both':
                    shareaxis = {'sharex': ax_list[-1], 'sharey': ax_list[-1]}
                else:
                    assert False, "Wrong value for share_axes: " + str(share_axes) + ". Possible values: 'auto', 'none', 'both'"
            else:
                shareaxis = {}
            ax = plt.subplot(1, len(df_list), j+1, **shareaxis)
        elif layout == 'grid':
            cols = columns
            rws = rows
            shareaxis = {}
            num_plots = cols * rws
            assert num_plots == len(df_list), "Number of plots is not the same as the number of dataframes"
            ax = plt.subplot(rws, cols, j+1)
        else:
            assert False, "layout is either 'horizontal', 'vertical', or 'grid'"
        
        if debug: 
            print("Creating subplots with " + str(layout) + " layout with "+ str(cols) + " column(s) and " + str(rws) + " row(s)\n")
            print("share_axes ", share_axes, "shareaxis: ", *shareaxis, "type", type(shareaxis))
    
        if 'order' in df.columns:
            df.sort_values(by='order', inplace=True)
        else:
            df.sort_values(by=legend_cols, inplace=True)
            
        for t in df.itertuples(index=False):
            style = utils._make_line_args(props, t, df)
            add_global_style_if_needed(global_style, style, debug)
            if 'additional_style' in df.columns and t.additional_style != None:
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

def plot_vectors_separate_faded(df, props, legend_func=utils.make_legend_label, layout='vertical', columns=0, rows=0, share_axes='auto', fade_factor=0.3, remove_ticks=True, global_style=None, debug=False):
    """
    This is a modified version of the built-in plot_vectors_separate() function. 
    Instead of plotting one line per subplot, it plots all lines on all subplots, but
    each have just one line in focus (i.e. not faded).
    
    The dataframes can also contain 'additional_style' columns.
    
    Layout can be:
    - 'vertical' (default)
    - 'horizontal'
    - 'grid': need to specify number of columns and rows
    
    By default, in the horizontal and vertical layouts, the common axes are shared, so they zoom together.
    Turn this off with 'share_axes'.
    
    share_axes: 
    - 'auto' (default): the common axes are shared (x for vertical, y for horizontal); both for 'grid'
    - 'none': no shared axes
    - 'x': for 'grid' only
    - 'y': for 'grid' only
    ' 'both': both axes are shared
    
    remove_ticks: remove ticks and tick labels between subplots
    
    Use modifiedplot.postconfigure() with this
    """
    p = ideplot if chart.is_native_chart() else plt
    
    title = ""
    ax_list = []
    
    props['plot_function'] = inspect.currentframe().f_code.co_name
    
    def make_individual_legend(legend_cols, t, props):
        print("legend cols:", legend_cols)
        print("t:", t)
    
    for j in range(0, len(df)):

        def get_prop(k):
            return props[k] if k in props else None
    
        title_cols, legend_cols = utils.extract_label_columns(df, props)
        
        cols = 0
        rws = 0
        
        shareaxis = {}
        
        ax = None
        
        if layout == 'vertical':
            cols = 1
            rws = len(df)
            # shareaxis = 'shareax = ax'
            if len(ax_list) != 0:
                if share_axes == 'auto':
                    shareaxis = {'sharex': ax_list[-1]}
                elif share_axes == 'none':
                    shareaxis = {}
                elif share_axes == 'both':
                    shareaxis = {'sharex': ax_list[-1], 'sharey': ax_list[-1]}
                else:
                    assert False, "Wrong value for share_axes: " + str(share_axes) + ". Possible values: 'auto', 'none', 'both'"                
            else:
                shareaxis = {}
            ax = plt.subplot(len(df), 1, j+1, **shareaxis)
        elif layout == 'horizontal':
            cols = len(df)
            rws = 1
            if len(ax_list) != 0:
                if share_axes == 'auto':
                    shareaxis = {'sharey': ax_list[-1]}
                elif share_axes == 'none':
                    shareaxis = {}
                elif share_axes == 'both':
                    shareaxis = {'sharex': ax_list[-1], 'sharey': ax_list[-1]}
                else:
                    assert False, "Wrong value for share_axes: " + str(share_axes) + ". Possible values: 'auto', 'none', 'both'"
            else:
                shareaxis = {}
            ax = plt.subplot(1, len(df), j+1, **shareaxis)
        elif layout == 'grid':
            if columns == 0 or rows == 0:
                n = math.sqrt(len(df))
                if debug: print("sqrt(len(df)):", n)
                if n % 1 == 0:
                    cols = int(n)
                    rws = int(n)
            else:
                cols = columns
                rws = rows
            if len(ax_list) != 0:
                if share_axes == 'auto' or share_axes == 'both':
                    print("its auto or both")
                    shareaxis = {'sharex': ax_list[-1], 'sharey': ax_list[-1]}
                elif share_axes == 'x':
                    shareaxis = {'sharex': ax_list[-1]}
                elif share_axes == 'y':
                    shareaxis = {'sharey': ax_list[-1]}
                elif share_axes == 'none':
                    shareaxis = {}
                else:
                    assert False, "Wrong value for share_axes: " + str(share_axes) + ". Possible values: 'auto', 'none', 'x', 'y', 'both'"
            num_plots = cols * rws
            assert num_plots == len(df), "Number of plots is not the same as the length of dataframes"
            ax = plt.subplot(rws, cols, j+1, **shareaxis)
        else:
            assert False, "layout is either 'horizontal', 'vertical', or 'grid'"
        
        if debug: 
            print("Creating subplots with " + str(layout) + " layout with "+ str(cols) + " column(s) and " + str(rws) + " row(s)\n")
            print("share_axes ", share_axes, "shareaxis: ", shareaxis, "type", type(shareaxis))
    
        if 'order' in df.columns:
            df.sort_values(by='order', inplace=True)
        else:
            df.sort_values(by=legend_cols, inplace=True)
            
        # for Matplotlib version < 1.5
        # plt.gca().set_color_cycle(None)
        # for Matplotlib version >= 1.5
        ax.set_prop_cycle(None)
            
        for z, t in zip(range(0, len(df)), df.itertuples(index=False)):
            style = utils._make_line_args(props, t, df)
            add_global_style_if_needed(global_style, style, debug)
            if 'additional_style' in df.columns and t.additional_style != None:
                style_dict = eval(t.additional_style)
                for i in style_dict.items():
                    style[i[0]] = i[1]
            label = ""
            if z != j:
                c = mpl.colors.ColorConverter.to_rgb(style['color'])
                # if True: print('lightening color. style color: ', style['color'], 'c1: ', c)
                # scale_lightness(c, 0.1)
                # if True: print('c2: ', c)
                # style['color'] = hextriplet(c)
                # style['color'] = rgb2hex(*c)
                style['color'] = fade_color(c, fade_factor)
                style['zorder'] = -10
                # print("XXXXXXXX t", t)
            else:
                label=legend_func(legend_cols, t, props)
                # if True: print('c3: ', style['color'])
            # label=legend_func(legend_cols, t, props)
            # label=make_individual_legend(legend_cols, t, props)
            p.plot(t.vectime, t.vecvalue, label=label, **style)
        
        if j == 0:
            title = get_prop("title") or utils.make_chart_title(df, title_cols)

            #utils.set_plot_title(title)
            plt.suptitle(title)
        ax_list.append(ax)
        utils.preconfigure_plot(props)

    p.ylabel(utils.make_chart_title(df, ["title"]))
    fix_labels_for_subplots(ax_list, props, layout, cols, rws, remove_ticks, debug)
    props['layout'] = layout
    props['columns'] = cols
    props['rows'] = rws
    
    return ax_list

def fix_labels_for_subplots(ax_list, props, layout, columns=0, rows=0, remove_ticks=True, debug=False):
    
        def get_prop(k):
            return props[k] if k in props else None
    
        if debug: print("layout:", layout)
        # if layout == 'vertical':
        #     for a in ax_list[0:-1]:
        #         a.set_xlabel('')
        #     ax_list[-1].set_xlabel(props['xaxis_title'])
        # elif layout == 'horizontal':
        #     for a in ax_list[1:]:
        #         if debug: print("a:", a)
        #         a.set_ylabel('')
        #     ax_list[0].set_ylabel(props['yaxis_title'])
        # elif layout == 'grid':
            # for a, i in zip(ax_list[1:], range(0, len(ax_list))):
            #     for col, row in zip(range(1,columns+1), range(1, rows+1)):
            #         # if (row+1) % (col+1) == 1:
            #         #     print("col, row", col, row)
            #         if i % (row+1) == 1:
            #             print("y label should be", i)
            #         if i % (col+1) == 1:
            #             print("x", i)
            
        if layout == 'vertical':
            columns=1
            rows=len(ax_list)
        elif layout == 'horizontal':
            columns=len(ax_list)
            rows=1
        elif layout == 'grid':
            assert columns != 0 and rows != 0, "specify number of columns and rows in fix_labels_for_subplots()"

        if debug: print("fix_labels_for_subplots: number of subplots: ", len(ax_list))
        for a in range(0, len(ax_list)):
            ax_list[a].set_xlabel("")
            ax_list[a].set_ylabel("")
            if remove_ticks:
                ax_list[a].tick_params(bottom=False, labelbottom=False)
                ax_list[a].tick_params(left=False, labelleft=False)
            if a%(len(ax_list)/rows) == 0:
                if debug: print("a", a, 'add y label')
                ax_list[a].set_ylabel(get_prop("yaxis_title"))
                ax_list[a].tick_params(left=True, labelleft=True)
            if a>=(len(ax_list)-columns):
                if debug: print("a", a, 'add x label')
                ax_list[a].set_xlabel(get_prop("xaxis_title"))
                ax_list[a].tick_params(bottom=True, labelbottom=True)

            if debug: print("a:", a)
                # a.set_ylabel('')
            # ax_list[0].set_ylabel(props['yaxis_title'])
            # ax_list[-1].set_xlabel(props['xaxis_title'])
            
        # else:
        #     assert False, "somethings wrong"            

def postconfigure_plot(props, debug=False):
    """
    Configures the plot according to the given properties, which normally
    get their values from setting in the "Configure Chart" dialog.
    Calling this function after plotting was performed should be a standard part
    of chart scripts.

    A partial list of properties taken into account:
    - `yaxis_title`, `yaxis_title`, `xaxis_min`,  `xaxis_max`, `yaxis_min`,
      `yaxis_max`, `xaxis_log`, `yaxis_log`, `legend_show`, `legend_border`,
      `legend_placement`, `grid_show`, `grid_density`
    - properties listed in the `plot.properties` property

    Parameters:
    - `props` (dict): the properties
    """
    p = ideplot if ideplot.is_native_plot() else plt

    def get_prop(k):
        return props[k] if k in props else None

    def setup():
        if 'plot_function' in props and props['plot_function'] != 'plot_vectors_separate_faded':
            if get_prop("xaxis_title"):
                p.xlabel(get_prop("xaxis_title"))
            if get_prop("yaxis_title"):
                p.ylabel(get_prop("yaxis_title"))
        # else:
        #     layout = props['layout']
        #     columns = props['columns']
        #     rows = props['rows']
        #     fix_labels_for_subplots(ax_list, props, layout, columns, rows, debug)
            

        if get_prop("xaxis_min"):
            p.xlim(left=float(get_prop("xaxis_min")))
        if get_prop("xaxis_max"):
            p.xlim(right=float(get_prop("xaxis_max")))
        if get_prop("yaxis_min"):
            p.ylim(bottom=float(get_prop("yaxis_min")))
        if get_prop("yaxis_max"):
            p.ylim(top=float(get_prop("yaxis_max")))

        if get_prop("xaxis_log"):
            p.xscale("log" if utils._parse_optional_bool(get_prop("xaxis_log")) else "linear")
        if get_prop("yaxis_log"):
            p.yscale("log" if utils._parse_optional_bool(get_prop("yaxis_log")) else "linear")

        ideplot.grid(utils._parse_optional_bool(get_prop("grid_show")),
            "major" if (get_prop("grid_density") or "").lower() == "major" else "both") # grid_density is "Major" or "All"

    if ideplot.is_native_plot():
        setup()

        ideplot.legend(show=utils._parse_optional_bool(get_prop("legend_show")),
           frameon=utils._parse_optional_bool(get_prop("legend_border")),
           loc=get_prop("legend_placement"))

        ideplot.set_properties(utils.parse_rcparams(get_prop("plot.properties") or ""))
    else:
        for ax in p.gcf().axes:
            plt.sca(ax)
            setup()

            if utils._parse_optional_bool(get_prop("legend_show")):

                loc = get_prop("legend_placement")
                args = { "loc": loc}
                if loc and loc.startswith("outside"):
                    args.update(_legend_loc_outside_args(loc))
                args["frameon"] =utils._parse_optional_bool(get_prop("legend_border"))
                plt.legend(**args)
        plt.tight_layout()

def add_to_dataframe(df, style_tuple_list=None, default_dict=None, order=None, debug=False):
    """
    Adds 'additional_style' column to dataframe. The concent of this column is added to 'style' object when plotting.
    Can also specify row order in dataframe (e.g. for ordering items in legend).
    style_tuple_list: [(column, value, {style dictionary}), (...), ...]:
        add style dictionary concents to 'additional_style' column in rows where column=value
    default_dict: {style dictionary}:
        add style to rows not matched by the above
    order: {'configname': order, ...}
        2-member list: first one is the column in the dataframe to order by, second is a dict with the order numbers
    or alternatively:
    order: ['configname', 'config1', 'config4', ... ]
        a list: the first item is the column in the dataframe to order by, the rest are the column values in order
        
    example:
    style_tuple_list = [('legend', 'eth[0]', {'linestyle': '--', 'linewidth': 2}), ('legend', 'eth[1]', {'linestyle': '-', 'linewidth': 2, 'marker': 's', 'markersize': 4})]
    default_dict = {'linestyle': '-', 'linewidth': 1}
    order = ['configname', {'Default_config': 1, 'Advanced_config': 0, 'Manual_config': 2}]
        or
    order = ['configname', 'Advanced_config', 'Default_config', 'Manual_config']
    
    Note that the value parameter in style_tuple_list and the order can contain regex (e.g. .*foo). Make sure to escape regex characters such as [ and ] with \
    """
    
    def add_separator():
        print("")
    def remove_zero_index_if_present(df, debug):
        remove = True
        if debug: print("remove zero index if present df:", df)
        for i, row in df.iterrows():
            if (i != 0):
                remove = False
                break
        if remove:
            df.reset_index(drop=True, inplace=True)
            if debug: print("removing zero index\n")
            
    remove_zero_index_if_present(df, debug)
    
    df['additional_style'] = None
    
    if default_dict is not None:    
        for i in range(0,len(df)):
#            if (df['additional_style'][i] == None):
            if debug: print('adding default stuff')
            df['additional_style'][i] = str(default_dict)
            
    if style_tuple_list is not None:
        for i in style_tuple_list:
            column = i[0]
            value = i[1]
            style_tuple = i[2]
            
            for i in range(0,len(df)):
                pattern = re.compile(value)
                if debug: print("value:", value, "pattern:", pattern, "df[column][i]", df[column][i])
                match = re.fullmatch(pattern, df[column][i])
                if debug: print("match:", match)
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
            
    if order is not None:
        # order the dataframe
        df['order'] = None
        
        if type(order[1]) is dict:
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
        else:
            order_column = order[0]
            order_list = order[1:]
            if debug: print('order_column:', order_column, '\norder_list', order_list)
            for order in range(0, len(order_list)):
                for j in range(0,len(df)):
                    # if df[order_column][j] == i[0]:
                    pattern = re.compile(order_list[order])
                    match = re.fullmatch(pattern, df[order_column][j])
                    if match != None:
                        df['order'][j] = order

                    
        if debug: 
            print("order added:")
            print(df['order'], df)

    return df

def create_multidimensional_legend(style_tuple_list, labels = [], handles = [], debug=False):
    """
    DEPRECATED (use create_custom_legend())
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
    print("create_multidimensional_legend() deprecated, use create_custom_legend()\n")
    return handles, labels

def quick_reorder_legend(handles, labels, order, **kwargs):
    """
    DEPRECATED (use create_custom_legend())
    Quickly reorder the legend. Order: list of integers with the new order, e.g. [2, 0, 1].
    Reorders the legend in-place (so it's not needed to call plt.legend() with the returned handles and labels).
    Also, add any parameters for plt.legend() as kwargs.
    """
    legendsize = len(handles)
    if order == []:
        order = [*range(0, legendsize)]
    plt.legend([handles[i] for i in order], [labels[i] for i in order], **kwargs)
    print("quick_reorder_legend() deprecated, use create_custom_legend()\n")
    return handles, labels

def create_custom_legend(props, style_tuple_list = None, order = None, labels = None, handles = None, inplace=True, debug=False, ax_list=None, **kwargs):
    """
    Can create multi-dimensional legend, where one aspect of a line (e.g. color) represents a dimension, another aspect (e.g. linestyle of solid, dashed or dotted) represents another dimension,
    as opposed to the default behavior, in which lines in the legend represent the lines on the chart directly. **TODO** not sure this explanation is needed
    
    If existing legend handles and labels are supplied arguments, the new legend entries are appended.
    
    style_tuple_list: [('legend label 1', {dict contaning the style arguments for the legend}), ('legend label 2, {...}), ...]
    
    for example:
    style_tuple_list = [('Label1', {'color': 'orange', 'linestyle': '--', 'markersize': 3}),
                        ('Label2', {'color': 'red', 'markersize': 3})]
                        
    Reorder the legend by specifying 'order'. Order: list of integers with the new order, e.g. [2, 0, 1].
    Also, add any parameters for plt.legend() as kwargs.
                        
    By default, does everything in-place (so it's not needed to call plt.legend() with the returned handles and labels).
    Returns legend handles and labels that can be supplied to plt.legend()
    """
    
    if style_tuple_list is None:
        style_tuple_list = []
    
    if order is None:
        order = []
        
    if labels is None:
        labels = []
        
    if handles is None:
        handles = []
        
    if props is None:
        props = {}
        
    if ax_list is None:
        ax_list = []

    if debug: 
        print("style_tuple_list", style_tuple_list)
        print("labels", labels, "handles", handles)
    for i in style_tuple_list:
        if debug: print("i", i)
        labels.append(i[0])
        if debug: print("i[0]", i[0], "i[1]", i[1])
        handles.append(Line2D([0], [0], **i[1]))
    if debug:
        print("labels", labels)
        print("handles", handles)
        
    if order == []:
        legendsize = len(handles)
        if debug: print("legendsize", legendsize)
        order = [*range(0, legendsize)]
        if debug: print("order:", order)
    if inplace:
        if 'plot_vectors_separate_grouped' in props.values():
            if debug: print("plot function is plot_vectors_separate_grouped")
            assert len(ax_list) > 0, "error: specify ax_list"
            for ax in ax_list:
                ax.legend([handles[i] for i in order], [labels[i] for i in order], **kwargs)
        else:
            plt.legend([handles[i] for i in order], [labels[i] for i in order], **kwargs)
    return handles, labels
            
def annotate_barchart(ax, offset=0, color='white', size=12, zorder=10, prefix='', postfix='', accuracy=2, outliers=None, result_type='float', debug=False):
    """
    Add text annotations to barcharts, displaying the bar height.
    
        Notes on parameters:
            - ax: axis object of the plot (add ax = plt.gca() before calling)
            - accuracy: number of decimal places in the annotation
            - outliers: 
                some bars might be too small to add the annotation. in this case, can place annotations above, for example.
                the parameter is a list of dicts, where each dict is like the following:
                    {'index': 2, 'color': 'black', ...}
                        ^ index of bar
                                         ^ style (has the same parameters)
            - result_type: set to 'int' to display results in the annotation without decimals (e.g. display 1024 instead of 1024.0)
    """
    if outliers == None:
        outliers = [{}]
        
    def is_in_outliers(index: int):
        for i in outliers:
            if debug: print("index " + str(index) + " is ")
            if 'index' in i and i['index'] == index:
                if debug: print("in outliers")
                return True
            else:
                if debug: print("not in outliers")
                return False
    
    for p in range(0, len(ax.patches)):
        if debug: print("p (ax.patches): ", p, ax.patches[p])
        if is_in_outliers(p):
            for i in outliers:
                if i['index'] == p:
                    i.setdefault('offset', 0)
                    i.setdefault('color', 'white')
                    i.setdefault('size', 12)
                    if debug: print("adding outlier stuff")
                    type_conv = eval(result_type)
                    if debug: print("type_conv: ", type_conv)
                    ax.annotate(prefix + str(round(type_conv(ax.patches[p].get_height()), accuracy)) + postfix, (ax.patches[p].get_x() + ax.patches[p].get_width() / 2, ax.patches[p].get_height() - i['offset']),
                                horizontalalignment='center', verticalalignment='top', color=i['color'], size=i['size'],
                                zorder=zorder)
        else:
            if debug: print('adding default stuff')
            type_conv = eval(result_type)
            if debug: print("type_conv: ", type_conv)
            ax.annotate(prefix + str(round(type_conv(ax.patches[p].get_height()), accuracy)) + postfix, (ax.patches[p].get_x() + ax.patches[p].get_width() / 2, ax.patches[p].get_height() - offset),
            horizontalalignment='center', verticalalignment='top', color=color, size=size,
            zorder=zorder)
                    
def plot_bars(df, errors_df=None, meta_df=None, props=None, order=None, zorder=None, rename=None, override_width=None, debug=False):
    """
    Modified version of the built-in plot_bars() function.
    The bars can be reordered with the optional 'order' argument. Also, can specify a z-order value, and rename bars with 'rename'.
    
    order: a list of column names in valuedf (for example, order = ['DefaultConfig', 'BasicConfig', 'AdvancedConfig']
    zorder: can change the z-order of the bars (0 by default)
    rename: a dictionary of column names containing what to rename to what (e.g. rename={'BasicConfig': 'Basic config', 'AdvancedConfig': 'Advanced config'}).
            this is applied after reordering
    
    original description:
    
        Creates a bar plot from the dataframe, with styling and additional input
        coming from the properties. Each row in the dataframe defines a series.
    
        Group names (displayed on the x axis) are taken from the column index.
    
        The name of the variable represented by the values can be passed in as
        the `variable_name` argument (as it is not present in the dataframe); if so,
        it will become the y axis label.
    
        Error bars can be drawn by providing an extra dataframe of identical
        dimensions as the main one. Error bars will protrude by the values in the
        errors dataframe both up and down (i.e. range is 2x error).
    
        To make the legend labels customizable, an extra dataframe can be provided,
        which contains any columns of metadata for each series.
    
        Colors are assigned automatically. The `cycle_seed` property allows you
        to select other combinations if the default one is not suitable.
    
        Parameters:
    
        - `df`: the dataframe
        - `props` (dict): the properties
        - `variable_name` (string): The name of the variable represented by the values.
        - `errors_df`: dataframe with the errors (in y axis units)
        - `meta_df`: dataframe with the metadata about each series
    
        Notable properties that affect the plot:
        - `baseline`: The y value at which the x axis is drawn.
        - `bar_placement`: Selects the arrangement of bars: aligned, overlap, stacked, etc.
        - `xlabel_rotation`: Amount of counter-clockwise rotation of x axis labels a.k.a. group names, in degrees.
        - `title`: Plot title (autocomputed if missing).
        - `cycle_seed`: Alters the sequence in which colors are assigned to series.
    """
    p = ideplot if ideplot.is_native_plot() else plt
    
    if props is None:
        props = {}

    def get_prop(k):
        return props[k] if k in props else None

    group_fill_ratio = 0.8
    aligned_bar_fill_ratio = 0.9
    overlap_visible_fraction = 1 / 3

    # used only by the mpl charts
    xs = np.arange(len(df.columns), dtype=np.float64)  # the x locations for the groups
    width = group_fill_ratio / len(df.index)  # the width of the bars
    bottoms = np.zeros_like(xs)
    stacks = np.zeros_like(xs)
    group_increment = 0.0

    baseline = get_prop("baseline")
    if baseline:
        if ideplot.is_native_plot(): # is this how this should be done?
            ideplot.set_property("Bars.Baseline", baseline)
        else:
            bottoms += float(baseline)

    extra_args = dict()

    placement = get_prop("bar_placement") or "Aligned"
    if placement:
        if ideplot.is_native_plot(): # is this how this should be done?
            ideplot.set_property("Bar.Placement", placement)
        else:
            if placement == "InFront":
                width = group_fill_ratio
            elif placement == "Stacked":
                width = group_fill_ratio
                bottoms *= 0 # doesn't make sense
            elif placement == "Aligned":
                width = group_fill_ratio / len(df.index)
                group_increment = width
                xs -= width * (len(df.index)-1)/2
                width *= aligned_bar_fill_ratio
            elif placement == "Overlap":
                width = group_fill_ratio / (1 + len(df.index) * overlap_visible_fraction)
                group_increment = width * overlap_visible_fraction
                extra_parts = (1.0 / overlap_visible_fraction - 1)
                xs += width / extra_parts - (len(df.index) + extra_parts) * width * overlap_visible_fraction / 2

    if order is None:
        df.sort_index(axis="columns", inplace=True)
        df.sort_index(axis="index", inplace=True)

    if errors_df is not None:
        if order is None:
            errors_df.sort_index(axis="columns", inplace=True)
            errors_df.sort_index(axis="index", inplace=True)

        assert(df.columns.equals(errors_df.columns))
        assert(df.index.equals(errors_df.index))

    title_cols, legend_cols = utils.extract_label_columns(meta_df.reset_index(), props)
    
    if order is not None:
        df = df.reindex(order, axis='columns')
        
    if rename is not None:
        df.rename(columns=rename, inplace=True)

    for i, ((index, row), meta_row) in enumerate(zip(df.iterrows(), meta_df.reset_index().itertuples(index=False))):
        style = utils._make_bar_args(props, df)

        if not ideplot.is_native_plot():
            if zorder is None: # FIXME: noot pretty...
                extra_args['zorder'] = 1 - (i / len(df.index) / 10)
            else:
                extra_args['zorder'] = zorder
            if override_width is not None:
                width = override_width
                if debug: print("override_width:", width)
            extra_args['bottom'] = bottoms + stacks

        label = utils.make_legend_label(legend_cols, meta_row, props)
        ys = row.values
        p.bar(xs, ys-bottoms, width, label=label, **extra_args, **style)

        if not ideplot.is_native_plot() and errors_df is not None and not errors_df.iloc[i].isna().all():
            plt.errorbar(xs, ys + stacks, yerr=errors_df.iloc[i], capsize=float(get_prop("cap_size") or 4), **style, linestyle="none", ecolor=mpl.rcParams["axes.edgecolor"])

        xs += group_increment
        if placement == "Stacked":
            stacks += row.values

    rotation = get_prop("xlabel_rotation")
    if rotation:
        rotation = float(rotation)
    else:
        rotation = 0
    p.xticks(list(range(len(df.columns))), list([utils._to_label(i) for i in df.columns.values]), rotation=rotation)

    # Add some text for labels, title and custom x-axis tick labels, etc.
    groups = df.columns.names

    p.xlabel(utils._to_label(groups))

    title = get_prop("title")
    if meta_df is not None:
        meta_df = meta_df.reset_index()
        if get_prop("legend_prefer_result_titles") == "true" and "title" in meta_df:
            series = meta_df["title"]
        else:
            series = meta_df["name"]

        title_names = series.unique()
        ylabel = title_names[0]
        if len(title_names) > 1:
            ylabel += ", etc."
        p.ylabel(ylabel)

        if title is None:
            title = utils.make_chart_title(meta_df, title_cols)

    if title is not None:
        utils.set_plot_title(title)
        
def plot_lines(df, props, legend_func=utils.make_legend_label, use_default_sort_values=False, global_style=None, debug=False):
    """
    Copy of built-in plot_lines() without sorting (by default).
    
    Orig description:
    
    Creates a line plot from the dataframe, with styling and additional input
    coming from the properties. Each row in the dataframe defines a line.

    Colors are assigned automatically.  The `cycle_seed` property allows you to
    select other combinations if the default one is not suitable.

    A function to produce the legend labels can be passed in. By default,
    `make_legend_label()` is used, which offers many ways to influence the
    legend via dataframe columns and chart properties. In the absence of
    more specified settings, the legend is normally computed from columns which best
    differentiate among the lines.

    Parameters:

    - `df`: The dataframe.
    - `props` (dict): The properties.
    - `legend_func` (function): The function to produce custom legend labels.
       See `utils.make_legend_label()` for prototype and semantics.

    Columns of the dataframe:

    - `x`, `y` (array-like, `len(x)==len(y)`): The X and Y coordinates of the points.
    - `error` (array-like, `len(x)==len(y)`, optional):
       The half lengths of the error bars for each point.
    - `legend` (string, optional): Legend label for the series. If missing,
       legend labels are derived from other columns.
    - `name`, `title`, `module`, etc. (optional): Provide input for the legend.

    Notable properties that affect the plot:

    - `title`: Plot title (autocomputed if missing).
    - `linewidth`: Line width.
    - `marker`: Marker style.
    - `linestyle`, `linecolor`, `linewidth`: Styling.
    - `error_style`: If `error` is present, controls how the error is shown.
       Accepted values: "Error bars", "Error band"
    - `cycle_seed`: Alters the sequence in which colors and markers are assigned to series.
    """
    p = ideplot if chart.is_native_chart() else plt

    def get_prop(k):
        return props[k] if k in props else None

    title_cols, legend_cols = utils.extract_label_columns(df, props)

    if use_default_sort_values:
        df.sort_values(by=legend_cols, inplace=True)
    if 'order' in df.columns:
        df.sort_values(by='order', inplace=True)
        if debug: print("values sorted by 'order' column")
    for t in df.itertuples(index=False):
        style = utils._make_line_args(props, t, df)
        add_global_style_if_needed(global_style, style, debug)

        if len(t.x) < 2 and style["marker"] == ' ':
            style["marker"] = '.'
            
        if 'additional_style' in df.columns:
            if debug: print("t.additional_style", t.additional_style)
            style_dict = eval(t.additional_style)
            # print("style_dict:", style_dict)
            for i in style_dict.items():
                style[i[0]] = i[1]

        p.plot(t.x, t.y, label=legend_func(legend_cols, t, props), **style)

        if hasattr(t, "error") and not ideplot.is_native_plot():
            style["linewidth"] = float(style["linewidth"])
            style["linestyle"] = "none"

            if props["error_style"] == "Error bars":
                plt.errorbar(t.x, t.y, yerr=t.error, capsize=float(props["cap_size"]), **style)
            elif props["error_style"] == "Error band":
                plt.fill_between(t.x, t.y-t.error, t.y+t.error, alpha=float(props["band_alpha"]))

    title = get_prop("title") or utils.make_chart_title(df, title_cols)
    utils.set_plot_title(title)
