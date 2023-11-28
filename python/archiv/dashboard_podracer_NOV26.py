#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import packages
from dash                   import Dash, html, dash_table, dcc, callback, Output, Input, ctx, State
from flask                  import Flask
import pandas               as pd
import plotly.express       as px
import plotly.graph_objects as go

import argparse
import dash_bootstrap_components as dbc


server = Flask(__name__)

app = Dash(__name__, server=server, external_stylesheets=[dbc.themes.BOOTSTRAP, dbc.icons.BOOTSTRAP])

#------------------------------------------------------------------------------------------------------
# Read csv data file
df = pd.read_csv(
#    "/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/data/students.csv",
    "/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/data/podrdata.csv",
    #dtype={'MS':int, 'TASK':str, 'GROUP':str},
    delimiter=',',
)

df["GROUPING"] = df.TASK + "_" + df.GROUP
df.set_index(["GROUPING"], inplace=True)

idx_time_min = df["TIME"].min()
idx_time_max = df["TIME"].max()

#------------------------------------------------------------------------------------------------------
# Build the components
header_component = html.H2("PODRacer's log visualizer", style={'color':'darkcyan'})

upload_component = dcc.Upload(
    id="upload-file",
    children=html.Div([
        html.A("Drag & Drop", style={'color':'red'}),
        " log file or ",
        html.A("Select a file", style={'color':'red'})
    ]),
    style={
        'width'         :'30%',
        'height'        : "30px",
        'lineHeigh'     : "30px",
        'borderWidth'   : "1px",
        'borderStyle'   : 'dashed',
        'borderRadius'  : '3px',
        'textAlign'     : 'center',
        'margin'        : '5px'
    },
    multiple=False
)

bottom_row = dbc.Row(
    [
        dbc.Col( upload_component)
    ]
)
#------------------------------------------------------------------------------------------------------
## user input widgets
dd_tasks = dcc.Dropdown(
            ["MIXER", "HOVER_UPD", "RECV_RD","RECV_WR", "SDIST", "OFLOW", "STEERING"],
            ["HOVER_UPD","RECV_WR"],
            multi=True, placeholder="Select tasks",
            id="dd-task"
        )

dd_channels = dcc.Dropdown(
            ["ROLL", "PITCH", "YAW","HOVER","THRUST"],
            ["ROLL", "HOVER", "YAW", "PITCH"],
            multi=True, placeholder="Select channels",
            id="dd-channels"
        )

dd_pids = dcc.Dropdown(
    {"PID_R":"ROLL", "PID_P":"PITCH", "PID_Y":"YAW","PID_H":"HOVER"},
    [],
    multi=True, placeholder="Select PID channels",
    id="dd-pid"
)

dd_floats = dcc.Dropdown(
            ["FLOAT0", "FLOAT1", "FLOAT2","FLOAT3","FLOAT4","FLOAT5","FLOAT6","FLOAT7"],
            [],
            multi=True, placeholder="Float values",
            id="dd-float"
        )

dd_longs = dcc.Dropdown(
            ["LONG0", "LONG1", "LONG2","LONG3","LONG4","LONG5","LONG6","LONG7"],
            [],
            multi=True, placeholder="Long values",
            id="dd-long"
        )

chk_graph = dcc.Checklist(
    options=[
        {'label':'Enable HoverMode', 'value':'hovermode'}
    ],
    value=[]
)

rs_time = dcc.RangeSlider(
            idx_time_min, idx_time_max, 500, id="rs-time", marks=None
        )

sl_spline = dcc.Slider(
    min=0.0, max=1.0, step=0.05,value=0.35,vertical=True, id="sl-spline"
)

#------------------------------------------------------------------------------------------------------
# Design the app
app.layout = html.Div(
    [
        # HEADERf
        dbc.Row(
            [
                header_component
            ]
        ),
        # User input for main figure
        dbc.Row([
                dbc.Col(
                    dbc.Button("Tasks & Channels", color="primary", id="btn-collapse-tasks-chnl", n_clicks=0),
                ),
                dbc.Col(
                    dbc.Button("PID Channels", color="primary", id="btn-collapse-pids", n_clicks=0),
                ),                
                dbc.Col(
                    dbc.Button("Floats & Longs", color="primary", id="btn-collapse-floats-longs", n_clicks=0),
                ),
                dbc.Col(
                    dbc.Button("Graph", color="primary", id="btn-collapse-graph", n_clicks=0),
                )
            ]
        ),
        dbc.Row([
                dbc.Collapse( 
                    dbc.Card([
                        dbc.Row([
                            dbc.Col([dd_tasks]),
                            dbc.Col([dd_channels])
                        ])
                    ],body=True),
                    id="collapse-tasks-channels",
                    is_open=False,
                ),
                dbc.Collapse( 
                    dbc.Card([
                        dbc.Row([
                            dbc.Col([dd_pids])
                        ])
                    ],body=True),
                    id="collapse-pids",
                    is_open=False,
                ),
                dbc.Collapse( 
                    dbc.Card([
                        dbc.Row([
                            dbc.Col([dd_floats]),
                            dbc.Col([dd_longs])
                        ])
                    ],body=True),
                    id="collapse-floats-longs",
                    is_open=False,
                ),
                dbc.Collapse( 
                    dbc.Card([
                        dbc.Row([
                            dbc.Col([chk_graph])
                        ])
                    ],body=True),
                    id="collapse-graph",
                    is_open=False,
                )
                ]
        ),
        # main figure        
        dbc.Row([
            dbc.Col(["Smooting",sl_spline],width=1),
            dbc.Col(dcc.Graph(id="fig-main"))
            ]),
        # user input for sub plots
        dbc.Row(
            [
            ]
        ),
        # sub plots
        dbc.Row(
            [
                dbc.Col(),
                dbc.Col(),
                dbc.Col()
            ]
        ),
        bottom_row


    ]

)

## Callbacks to control graphs
@callback(
     Output(component_id='fig-main', component_property='figure'),
     Input(component_id='sl-spline', component_property='value'),
     #Input(component_id='dd-tasks', component_property='value')
)
def update_main_graph_smoothing(smoothing_value):
    ## graphs
    fig_main = go.FigureWidget()
    fig_main.add_scatter(name="ROLL", x=df.TIME, y=df.CH_R, line_shape='spline', line={'smoothing':smoothing_value})
    fig_main.add_scatter(name="PITCH", x=df.TIME, y=df.CH_P, line_shape='spline', line={'smoothing':smoothing_value})
    fig_main.add_scatter(name="YAW", x=df.TIME, y=df.CH_Y, line_shape='spline', line={'smoothing':smoothing_value})
    fig_main.add_scatter(name="HOVER", x=df.TIME, y=df.CH_H, line_shape='spline', line={'smoothing':smoothing_value})
    fig_main.update_traces(mode="markers+lines", hovertemplate=None)
    fig_main.update_layout(
        title="Main graph",
        hovermode="x unified",
        xaxis=dict(
            rangeslider=dict(
                visible=True
            )
        )
    )
    return fig_main

## APP-Callbacks

@app.callback(
    Output("collapse-tasks-channels", "is_open"),
    Input("btn-collapse-tasks-chnl", "n_clicks"),
    [State("collapse-tasks-channels", "is_open")]
)
def toggle_user_input(n_click, is_open):
    if n_click:
        return not is_open
    return is_open

@app.callback(
    Output("collapse-floats-longs", "is_open"),
    Input("btn-collapse-floats-longs", "n_clicks"),
    [State("collapse-floats-longs", "is_open")]
)
def toggle_user_input(n_click, is_open):
    if n_click:
        return not is_open
    return is_open

@app.callback(
    Output("collapse-pids", "is_open"),
    Input("btn-collapse-pids", "n_clicks"),
    [State("collapse-pids", "is_open")]
)
def toggle_user_input(n_click, is_open):
    if n_click:
        return not is_open
    return is_open


@app.callback(
    Output("collapse-graph", "is_open"),
    Input("btn-collapse-graph", "n_clicks"),
    [State("collapse-graph", "is_open")]
)
def toggle_user_input(n_click, is_open):
    if n_click:
        return not is_open
    return is_open


# run the app
if __name__ == '__main__':
    app.run(debug=False)
