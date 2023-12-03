# Import packages
import dash
from dash                   import Dash, html, dash_table, dcc, callback, Output, Input, ctx, State
from flask                  import Flask
import plotly.express       as px
import plotly.graph_objects as go

import argparse
import dash_bootstrap_components as dbc
from modules.datasets import *
from modules.components import *
from pages.default_fig import default_fig
from dash_bootstrap_templates import ThemeSwitchAIO

template_theme2 = "flatly"
template_theme1 = "darkly"


dash.register_page(__name__, name="Static analysis", top_nav=True)


#------------------------------------------------------------------------------------------------------
# Design the app
layout = html.Div(
    [
        # HEADERf
        static_header_row,
        # User input for main figure
        # ----------------------------------------------------------------------
        # upper graph   
        # ----------------------------------------------------------------------
        dbc.Row([
            dbc.Col([dbc.Button(">>",id="btn-backdrop-1", n_clicks=0)],width=1),
            offcanvas,
            dbc.Col(dcc.Graph(id="fig-main", figure=default_fig))
            ], align="center"),
        # ----------------------------------------------------------------------
        # lower graph   
        # ----------------------------------------------------------------------
        dbc.Row([
            dbc.Col([dbc.Button(">>",id="btn-backdrop-2", n_clicks=0)],width=1),
            dbc.Col(dcc.Graph(id="fig-graph2", figure=default_fig))
            ], align="center"),
        # ----------------------------------------------------------------------
        # Bottom-Row  
        # ----------------------------------------------------------------------
    ]

)

def pandas_rename_column(x,g="?"):
    if x == 'TIME' or x == 'TASK' or x == 'GROUP':
        return x
    return f"{g}_{x}"

#------------------------------------------------------------------------------------------
# Callbacks
#------------------------------------------------------------------------------------------

#**********************************************
# Create & Callback for fig_main (upper graph)
#**********************************************
def _update_fig_main(smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels, toggle):
    ## graphs
    df_filt_tasks = None
    df_t = []

    # for every task we need a separate grouped dataset
    for idx, t in enumerate(tasks):
        df_t.append(df.loc[df.index.get_level_values('GROUPING') == tasks[idx]])
        df_filt_tasks = pd.concat([df_filt_tasks, df_t[idx]],axis=0)

    fig_main = go.FigureWidget()
    #
    # build now für every task / every channel a separate scatter-plot
    for ic, c in enumerate(channels):
        for ig, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            # building a access to the yaxis value for later evaluation
            # Note: the time basis (xaxis is allways the same)
            ydata = f"df_t[{ig}].{c}"
            legend = f"{c}_{str(g)}"
            fig_main.add_scatter(name=legend, x=df_t[ig].TIME, y=eval(ydata), line_shape='spline', line={'smoothing':smoothing_value})        

    #
    #
    
    template = template_theme1 if toggle else template_theme2
    fig_main.update_xaxes(title="milliseconds", type='linear' if xaxis_type == 'Linear' else 'log')
    fig_main.update_yaxes(title="sbus-value", type='linear' if yaxis_type == 'Linear' else 'log')
    fig_main.update_layout(
        title="Main graph",
        #xaxis_tickformat='ms',
        template=template,
        xaxis=dict(
            dtick=500,
            rangeslider=dict(
                visible=True
            )
        ),
        yaxis=dict(
            dtick = 200
        )
    )
    if len(hover_mode) > 0:
        fig_main.update_traces(mode="markers+lines", hovertemplate=None)
        fig_main.update_layout(
            hovermode="x unified"
        )

    return fig_main


def _update_fig_graph2(smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels):
    ## graphs
    df_filt_tasks = None
    df_t = []

    # for every task we need a separate grouped dataset
    for idx, t in enumerate(tasks):
        df_t.append(df.loc[df.index.get_level_values('GROUPING') == tasks[idx]])
        df_filt_tasks = pd.concat([df_filt_tasks, df_t[idx]],axis=0)

    fig = go.FigureWidget()
    #
    # build now für every task / every channel a separate scatter-plot
    for ic, c in enumerate(channels):
        for ig, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            # building a access to the yaxis value for later evaluation
            # Note: the time basis (xaxis is allways the same)
            ydata = f"df_t[{ig}].{c}"
            legend = f"{c}_{str(g)}"
            fig.add_scatter(name=legend, x=df_t[ig].TIME, y=eval(ydata), line_shape='spline', line={'smoothing':smoothing_value})        

    #
    #
    fig.update_xaxes(title="milliseconds", type='linear' if xaxis_type == 'Linear' else 'log')
    fig.update_yaxes(title="sbus-value", type='linear' if yaxis_type == 'Linear' else 'log')
    fig.update_layout(
        title="Sub-Graph",
        #xaxis_tickformat='ms',
        xaxis=dict(
            dtick=500,
            rangeslider=dict(
                visible=True
            )
        ),
        yaxis=dict(
            dtick = 200
        )
    )
    if len(hover_mode) > 0:
        fig.update_traces(mode="markers+lines", hovertemplate=None)
        fig.update_layout(
            hovermode="x unified"
        )

    return fig

@callback(
    Output(component_id='fig-main', component_property='figure'),
    Input(component_id="dd-graph-select", component_property='value'),
    Input(component_id='sl-spline', component_property='value'),
    Input(component_id='chk-hover-mode', component_property='value'),
    Input(component_id='rb-xaxis-type', component_property='value'),
    Input(component_id='rb-yaxis-type', component_property='value'),
    Input(component_id='dd-task-filter', component_property='value'),
    Input(component_id='dd-channels-filter', component_property='value'),
    Input(ThemeSwitchAIO.ids.switch("theme"), "value"),    
)
def update_main(graph, smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels, toggle):
    if graph == "fig-main":
        fig = _update_fig_main(smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels, toggle)
    return fig

#**********************************************
# Create & Callback for fig_graph2
#**********************************************

@callback(
    Output(component_id='fig-graph2', component_property='figure'),
    Input(component_id="dd-graph-select", component_property='value'),
    Input(component_id='sl-spline', component_property='value'),
    Input(component_id='chk-hover-mode', component_property='value'),
    Input(component_id='rb-xaxis-type', component_property='value'),
    Input(component_id='rb-yaxis-type', component_property='value'),
    Input(component_id='dd-task-filter', component_property='value'),
    Input(component_id='dd-channels-filter', component_property='value'),
)
def update_graph2(graph, smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels):
    fig = None
    if graph == "fig-graph2":
        fig = _update_fig_graph2(smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels)
    return fig
