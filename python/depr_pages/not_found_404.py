from dash import html
import dash

dash.register_page(__name__, path="/404")


layout = html.H1("ups - this page was not found 404")