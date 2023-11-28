from dash                   import Dash, html, dash_table, dcc, callback, Output, Input, ctx, State
from flask                  import Flask
from modules.datasets     import *

import dash_bootstrap_components    as dbc
import pandas                       as pd
import plotly.express               as px
import plotly.graph_objects         as go


#------------------------------------------------------------------------------------------------------
# Build the components
#
# This file contain all components which are used in the application
#------------------------------------------------------------------------------------------------------

""" define upload component """
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

""" for future use Live-data button. per default deactivated. """
btn_live = dbc.Button(
    "LIVE-DATA", id="btn-live-data", n_clicks=0, outline=True, color="secondary", disabled=True
)

#------------------------------------------------------------------------------------------------------
# Modal pop up windows 
#------------------------------------------------------------------------------------------------------
""" modal pop up window if live-button was clicked """
modal_live = dbc.Modal(
    [
        dbc.ModalHeader(dbc.ModalTitle("LIVE DATA collection")),
        dbc.ModalBody("Before you start, please start 'serial_reader.py', than click button start"),
        dbc.ModalFooter(
            [
                dbc.Button(
                    "Close", id="close-live", className="ms-auto", n_clicks=0
                ),
                dbc.Button(
                    "Run", id="run-live", color="success", className="ms-auto", n_clicks=0
                )
            ]    
        ),
    ],
    id="modal-live",
    is_open=False,
)

""" the complete header row. """
header_row = dbc.Row(
    [
        dbc.Col(html.H2("PODRacer's log visualizer", style={'color':'darkcyan'}), width=6),
        dbc.Col(btn_live, width=2),
        modal_live,
        dbc.Col(upload_component, width=4)
    ]
)

""" the complete bottom row (currently not used)"""
bottom_row = dbc.Row(
    [
        dbc.Col( upload_component)
    ]
)

#------------------------------------------------------------------------------------------------------
# RadioItems components
#------------------------------------------------------------------------------------------------------
""" xaxis data publishing as linear or logirthm """
rb_xaxis_type = dcc.RadioItems(
    ["Linear", "Log"],
    "Linear",
    id="rb-xaxis-type",
    labelStyle={'display': 'inline-block', 'marginTop': '3px'}
)

""" yaxis data publishing as linear or logirthm """
rb_yaxis_type = dcc.RadioItems(
    ["Linear", "Log"],
    "Linear",
    id="rb-yaxis-type",
    labelStyle={'display': 'inline-block', 'marginTop': '3px'}
)

#------------------------------------------------------------------------------------------------------
# Labels components
#------------------------------------------------------------------------------------------------------
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

# rb_yaxis_type = dcc.RadioItems(
#     ["Linear", "Log"],
#     ["Linear"],
#     id="rb-yaxis-type",
#     labelStyle={'display': 'inline-block', 'marginTop': '5px'}
# )

#------------------------------------------------------------------------------------------------------
# Dropdown components 
#------------------------------------------------------------------------------------------------------
dd_tasks = dcc.Dropdown(
            ["MIXER", "HOVER_UPD", "RECV_RD","RECV_WR", "SDIST", "OFLOW", "STEERING"],
            ["RECV_WR", "HOVER_UPD"],
            multi=True, placeholder="Select tasks",
            id="dd-task-filter"
        )

dd_channels = dcc.Dropdown(
    {
        'CH_R':'ROLL',
        'CH_P':'PITCH',
        'CH_Y':'YAW',
        'CH_H':'HOVER',
        'CH_T':'THRUST',
        'AUX2':'AUX2',
        'AUX3':'AUX3',
     
    },
    ["CH_H"],
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


#------------------------------------------------------------------------------------------------------
# Checkboxes components
#------------------------------------------------------------------------------------------------------

chk_hover_mode = dcc.Checklist(
    options=[
        {'label':'Enable HoverMode', 'value':'hovermode'}
    ],
    id="chk-hover-mode",
    value=[]
)

chk_enable_live = dcc.Checklist(
    options=[
        {'label':'Live-Mode', 'value':True}
    ],
    id="chk-live-mode",
    value=[]
)


#------------------------------------------------------------------------------------------------------
# Slider componentes
#------------------------------------------------------------------------------------------------------

rs_time = dcc.RangeSlider(
            idx_time_min, idx_time_max, 500, id="rs-time", marks=None
        )

sl_spline = dcc.Slider(
    min=0.0, max=0.75, step=(0.75/10),value=0.375, id="sl-spline"
)

#------------------------------------------------------------------------------------------------------
# offcanvas components
#------------------------------------------------------------------------------------------------------
# open on the left side - this canva is used twice - for upper graph and lower graph  
offcanvas = dbc.Offcanvas(
    [
        dbc.Row([
            dbc.Col([chk_hover_mode]),
            dbc.Col([chk_enable_live])
        ]),
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
        dbc.Row([dbc.Label("Choose PIDs"), dbc.Col(dd_pids)]),
        dbc.Row([dbc.Label("Choose floats"), dbc.Col(dd_floats)]),
        dbc.Row([dbc.Label("Choose longs"), dbc.Col(dd_longs)])
    ],
    id="offcanvas-graph-content",
    scrollable=True,
    is_open=False
)
