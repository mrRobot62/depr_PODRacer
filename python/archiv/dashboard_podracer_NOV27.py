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

upload_component = dcc.Upload(
    id="upload-file",
    children=html.Div([
        html.A("Drag & Drop", style={'color':'red'}),
        " log file or ",
        html.A("Select a file", style={'color':'red'})
    ]),
    style={
        'width'         :'100%',
        'height'        : "35px",
        'lineHeigh'     : "35px",
        'borderWidth'   : "1px",
        'borderStyle'   : 'dashed',
        'borderRadius'  : '5px',
        'textAlign'     : 'center',
        'margin'        : '5px'
    },
    multiple=False
)

header_row = dbc.Row(
    [
        dbc.Col(html.H2("PODRacer's log visualizer", style={'color':'darkcyan'}), width=8),
        dbc.Col(upload_component, width=4)
    ]
)

bottom_row = dbc.Row(
    [
        dbc.Col( upload_component)
    ]
)

#------------------------------------------------------------------------------------------------------
## user input widgets
rb_xaxis_type = dcc.RadioItems(
    ["Linear", "Log"],
    "Linear",
    id="rb-xaxis-type",
    labelStyle={'display': 'inline-block', 'marginTop': '3px'}
)

rb_yaxis_type = dcc.RadioItems(
    ["Linear", "Log"],
    "Linear",
    id="rb-yaxis-type",
    labelStyle={'display': 'inline-block', 'marginTop': '3px'}
)

rb_xaxis_type_comp = dbc.Row(
    [
        dbc.Label("XAxis-Type"), rb_xaxis_type
    ]
)
rb_yaxis_type_comp = dbc.Row(
    [
        dbc.Label("YAxis-Type"), rb_yaxis_type
    ]
)

rb_yaxis_type = dcc.RadioItems(
    ["Linear", "Log"],
    ["Linear"],
    id="rb-yaxis-type",
    labelStyle={'display': 'inline-block', 'marginTop': '5px'}
)

dd_tasks = dcc.Dropdown(
            ["MIXER", "HOVER_UPD", "RECV_RD","RECV_WR", "SDIST", "OFLOW", "STEERING"],
            ["RECV_WR"],
            multi=True, placeholder="Select tasks",
            id="dd-task-filter"
        )

dd_channels = dcc.Dropdown(
            ["ROLL", "PITCH", "YAW","HOVER","THRUST"],
            ["ROLL", "HOVER", "YAW", "PITCH"],
            multi=True, placeholder="Select channels",
            id="dd-channels-filter"
        )

dd_pids = dcc.Dropdown(
    {"PID_R":"ROLL", "PID_P":"PITCH", "PID_Y":"YAW","PID_H":"HOVER"},
    [],
    multi=True, placeholder="Select PID channels",
    id="dd-pid-filter"
)

dd_floats = dcc.Dropdown(
            ["FLOAT0", "FLOAT1", "FLOAT2","FLOAT3","FLOAT4","FLOAT5","FLOAT6","FLOAT7"],
            [],
            multi=True, placeholder="Float values",
            id="dd-float-filter"
        )

dd_longs = dcc.Dropdown(
            ["LONG0", "LONG1", "LONG2","LONG3","LONG4","LONG5","LONG6","LONG7"],
            [],
            multi=True, placeholder="Long values",
            id="dd-long-filter"
        )

chk_hover_mode = dcc.Checklist(
    options=[
        {'label':'Enable HoverMode', 'value':'hovermode'}
    ],
    id="chk-hover-mode",
    value=[]
)

rs_time = dcc.RangeSlider(
            idx_time_min, idx_time_max, 500, id="rs-time", marks=None
        )

sl_spline = dcc.Slider(
    min=0.0, max=0.75, step=(0.75/10),value=0.375, id="sl-spline"
)

# open on the left side - this canva is used twice - for upper graph and lower graph
#  
offcanvas = dbc.Offcanvas(
    [
        dbc.Row([dbc.Col([chk_hover_mode])]),
        html.Hr(),
        dbc.Row([
            dbc.Col([rb_xaxis_type_comp]), 
            dbc.Col([rb_yaxis_type_comp])]
        ),
        html.Hr(),
        dbc.Row(dbc.Col(["Smooting",sl_spline])),
        html.Hr(),
        dbc.Row([dbc.Label("Choose tasks"), dbc.Col(dd_tasks)]),
        dbc.Row([dbc.Label("Choose channels"), dbc.Col(dd_channels)]),
        dbc.Row([dbc.Label("Choose channels"), dbc.Col(dd_pids)]),
        dbc.Row([dbc.Label("Choose channels"), dbc.Col(dd_floats)]),
        dbc.Row([dbc.Label("Choose channels"), dbc.Col(dd_longs)])
    ],
    id="offcanvas-graph-content",
    scrollable=True,
    is_open=False
)

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
    if len(tasks) > 0:
        df_t.append(df.loc[df.index.get_level_values('GROUPING') == tasks[0]])
        df_t.append(df.loc[df.index.get_level_values('GROUPING') == tasks[1]] if len(tasks)==2  else None)
        df_t.append(df.loc[df.index.get_level_values('GROUPING') == tasks[2]] if len(tasks)==3  else None)
        df_t.append(df.loc[df.index.get_level_values('GROUPING') == tasks[3]] if len(tasks)==4  else None)

        if df_t[0] is not None:
            #df_t0.rename(colums=pandas_rename_column)
            df_filt_tasks = pd.concat([df_filt_tasks, df_t[0]], axis=0)
        if df_t[1] is not None:
            df_filt_tasks = pd.concat([df_filt_tasks, df_t[1]], axis=0)
        if df_t[2] is not None:
            df_filt_tasks = pd.concat([df_filt_tasks, df_t[2]], axis=0)
        if df_t[3] is not None:
            df_filt_tasks = pd.concat([df_filt_tasks, df_t[3]], axis=0)

        #df_filt_tasks = df.loc[df.index.get_level_values('GROUPING') == tasks[0]]

    fig_main = go.FigureWidget()
    if 'ROLL' in channels:
        for idx, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            fig_main.add_scatter(name=f"ROLL_{str(g)}", x=df_t[idx].TIME, y=df_t[idx].CH_R, line_shape='spline', line={'smoothing':smoothing_value})        
    if 'PITCH' in channels:
        for idx, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            fig_main.add_scatter(name=f"PITCH_{str(g)}", x=df_t[idx].TIME, y=df_t[idx].CH_P, line_shape='spline', line={'smoothing':smoothing_value})
    if 'YAW' in channels:
        for idx, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            fig_main.add_scatter(name=f"YAW_{str(g)}", x=df_t[idx].TIME, y=df_t[idx].CH_Y, line_shape='spline', line={'smoothing':smoothing_value})
    if 'HOVER' in channels:
        for idx, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            fig_main.add_scatter(name=f"HOVER_{str(g)}", x=df_t[idx].TIME, y=df_t[idx].CH_H, line_shape='spline', line={'smoothing':smoothing_value})
    if 'THRUST' in channels:
        for idx, g in enumerate(df_filt_tasks.index.get_level_values('GROUPING').unique()):
            fig_main.add_scatter(name=f"THRUST_{str(g)}", x=df_t[idx].TIME, y=df_t[idx].CH_T, line_shape='spline', line={'smoothing':smoothing_value})

    fig_main.update_xaxes(title="milliseconds", type='linear' if xaxis_type == 'Linear' else 'log')
    fig_main.update_yaxes(title="sbus-value", type='linear' if yaxis_type == 'Linear' else 'log')
    fig_main.update_layout(
        title="Main graph",
        xaxis=dict(
            rangeslider=dict(
                visible=True
            )
        )
    )
    if len(hover_mode) > 0:
        fig_main.update_traces(mode="markers+lines", hovertemplate=None)
        fig_main.update_layout(
            hovermode="x unified"
        )

    return fig_main

# Callbacks

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

# run the app
if __name__ == '__main__':
    app.run(debug=False)
