from dash import html
import dash

dash.register_page(__name__, name="Data-Table view", top_nav=True)


layout = html.Div("GRID page")