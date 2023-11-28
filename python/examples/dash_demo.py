# Import packages
from dash import Dash, html, dash_table, dcc, callback, Output, Input
import pandas as pd
import plotly.express as px

# Incorporate data
df = pd.read_csv(
    "/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/data/podrdata.csv"
)

# Initialize the app
app = Dash(__name__)

# App layout
app.layout = html.Div([
    html.Div(children='PODRacer Visualizer'),
    html.Hr(),
    dcc.RadioItems(options=['pop', 'lifeExp', 'gdpPercap'], value='lifeExp', id='my-final-radio-item-example'),
    dash_table.DataTable(data=df.to_dict('records'), page_size=6),
    dcc.Graph(figure={}, id='dash-demo')
])

# Add controls to build the interaction
@callback(
    Output(component_id='dash-demo', component_property='figure'),
    Input(component_id='dash-demo', component_property='value')
)
def update_graph(col_chosen):
    fig = px.histogram(df, x='continent', y=col_chosen, histfunc='avg')
    return fig

# Run the app
if __name__ == '__main__':
    app.run(debug=False, port=4050)
