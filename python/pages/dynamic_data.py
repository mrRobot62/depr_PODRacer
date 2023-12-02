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

template_theme2 = "flatly"
template_theme1 = "darkly"


dash.register_page(__name__, top_nav=True)


#------------------------------------------------------------------------------------------------------
# Design the app
layout = html.Div(

)
