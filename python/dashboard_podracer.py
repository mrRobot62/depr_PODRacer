#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import packages
from dash                   import Dash, html, dash_table, dcc, callback, Output, Input, ctx, State
from flask                  import Flask
import plotly.express       as px
import plotly.graph_objects as go

import argparse
import dash_bootstrap_components as dbc

server = Flask(__name__)

app = Dash(__name__, server=server, external_stylesheets=[dbc.themes.BOOTSTRAP, dbc.icons.BOOTSTRAP])

from modules.datasets import *
from modules.components import *

#------------------------------------------------------------------------------------------------------
# Design the app
app.layout = html.Div(
    [
        # HEADERf
        header_row,
        # User input for main figure
        # ----------------------------------------------------------------------
        # upper graph   
        # ----------------------------------------------------------------------
        dbc.Row([
            dbc.Col([dbc.Button(">>",id="btn-backdrop-1", n_clicks=0)],width=1),
            offcanvas,
            dbc.Col(dcc.Graph(id="fig-main"))
            ], align="center"),
        # ----------------------------------------------------------------------
        # lower graph   
        # ----------------------------------------------------------------------
        dbc.Row([
            dbc.Col([dbc.Button(">>",id="btn-backdrop-2", n_clicks=0)],width=1),
            #getOffCanvas(2),
            dbc.Col(dcc.Graph(id="fig-graph2"))
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


## Callbacks to control graphs
@callback(
    Output(component_id='fig-main', component_property='figure'),
    Input(component_id='sl-spline', component_property='value'),
    Input(component_id='chk-hover-mode', component_property='value'),
    Input(component_id='rb-xaxis-type', component_property='value'),
    Input(component_id='rb-yaxis-type', component_property='value'),
    Input(component_id='dd-task-filter', component_property='value'),
    Input(component_id='dd-channels-filter', component_property='value'),
)
def update_main_graph_smoothing(smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels):
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
    fig_main.update_xaxes(title="milliseconds", type='linear' if xaxis_type == 'Linear' else 'log')
    fig_main.update_yaxes(title="sbus-value", type='linear' if yaxis_type == 'Linear' else 'log')
    fig_main.update_layout(
        title="Main graph",
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
        fig_main.update_traces(mode="markers+lines", hovertemplate=None)
        fig_main.update_layout(
            hovermode="x unified"
        )

    return fig_main

# Callbacks
@callback(
    Output("btn-live-data","disabled"),
    Input("chk-live-mode", "value")    
)
def toggle_live_botton(enable_mode):
    if True in enable_mode:
        return False
    return True

## APP-Callbacks
@app.callback(
    Output("offcanvas-graph-content", "is_open"),
    Input("btn-backdrop-1", "n_clicks"), Input("btn-backdrop-2", "n_clicks"),
    State("offcanvas-graph-content", "is_open"),
)
def toggle_offcanvas(n1,n2, is_open):
    if n1 or n2:
        return not is_open
    return is_open


@app.callback(
    Output("modal-live", "is_open"),
    Input("btn-live-data", "n_clicks"), Input("close-live", "n_clicks"), Input("run-live", "n_clicks"),
    [State("modal-live", "is_open")],
)
def toggle_modal(n1, n2, n3, is_open):
    if n1 or n2:
        return not is_open
    return is_open

# run the app
if __name__ == '__main__':
    app.run(debug=False)
