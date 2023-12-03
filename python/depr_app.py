
# Import packages
from dash                   import Dash, html, dash, page_container, page_registry, dash_table, dcc, callback, Output, Input, ctx, State
from flask                  import Flask
import plotly.express       as px
import plotly.graph_objects as go

import argparse
import dash_bootstrap_components as dbc
from dash_bootstrap_templates import ThemeSwitchAIO

server = Flask(__name__)

# select the Bootstrap stylesheets and figure templates for the theme toggle here:
url_theme1 = dbc.themes.FLATLY
url_theme2 = dbc.themes.DARKLY
theme_toggle = ThemeSwitchAIO(
    aio_id="theme",
    themes=[url_theme2, url_theme1],
    icons={"left": "fa fa-sun", "right": "fa fa-moon"},
)


# This stylesheet defines the "dbc" class.  Use it to style dash-core-components
# and the dash DataTable with the bootstrap theme.
#dbc_css = "https://cdn.jsdelivr.net/gh/AnnMarieW/dash-bootstrap-templates/dbc.min.css"
dbc_css = "css/dbc.min.css"


from modules.datasets import *
from modules.components import *

AppName    = "PODRacer LogVisualizer"
AppVersion = "1.0.1"



app = Dash(__name__, server=server, 
           use_pages=True,
            external_stylesheets=[dbc.themes.BOOTSTRAP,
            dbc.icons.BOOTSTRAP]
            )


navbar = dbc.NavbarSimple(
    dbc.Nav(
        [ 
           dbc.NavLink(page["name"], href=page["path"])
            for page in page_registry.values()
            #if page["module"] != "pages.not_found_404"
            if page.get("top_nav") 
        ]
    ),
    brand=f"{AppName} V{AppVersion}",
    color="primary",
    dark=True,
    className="mb-2",
)

app.layout = dbc.Container(
    [navbar, 
     theme_toggle,
     dash.page_container],
    fluid=True,
)

#---------------------------------------------------------------
# callbacks on app-level
#---------------------------------------------------------------

#**********************************************
# Callbacks for Live-Modal
#**********************************************
"""

@callback(
    Output("btn-live-data","disabled"),
    Input("chk-live-mode", "value")    
)
def toggle_live_botton(enable_mode):
    if True in enable_mode:
        return False
    return True

@app.callback(
    Output("modal-live", "is_open"),
    Input("btn-live-data", "n_clicks"), Input("close-live", "n_clicks"), Input("run-live", "n_clicks"),
    [State("modal-live", "is_open")],
)
def toggle_modal(n1, n2, n3, is_open):
    if n1 or n2:
        return not is_open
    return is_open

"""


if __name__ == "__main__":
    app.run_server(debug=False)