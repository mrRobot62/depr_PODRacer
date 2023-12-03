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


dash.register_page(__name__, name="Live analysis", top_nav=True)


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
            dbc.Col(dcc.Graph(id="fig-live-main", figure=default_fig))
            ], align="center"),
        # ----------------------------------------------------------------------
        # lower graph   
        # ----------------------------------------------------------------------
        dbc.Row([
            dbc.Col([dbc.Button(">>",id="btn-backdrop-2", n_clicks=0)],width=1),
            dbc.Col(dcc.Graph(id="fig-live-sub1", figure=default_fig))
            ], align="center"),
        # ----------------------------------------------------------------------
        # Bottom-Row  
        # ----------------------------------------------------------------------
    ]

)

@callback(
    Output(component_id='fig-live-main', component_property='figure'),
    Input(component_id="dd-graph-select", component_property='value'),
    Input(component_id='sl-spline', component_property='value'),
    Input(component_id='chk-hover-mode', component_property='value'),
    Input(component_id='rb-xaxis-type', component_property='value'),
    Input(component_id='rb-yaxis-type', component_property='value'),
    Input(component_id='dd-task-filter', component_property='value'),
    Input(component_id='dd-channels-filter', component_property='value'),
    Input(ThemeSwitchAIO.ids.switch("theme"), "value"),    
)
def update_live_main(graph, smoothing_value, hover_mode, xaxis_type, yaxis_type, tasks, channels, toggle):
    return fig