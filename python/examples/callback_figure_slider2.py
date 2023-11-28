from dash import Dash, dcc, html, Input, Output, callback
import plotly.express as px

import pandas as pd

df = pd.read_csv('/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/examples/gapminderDataFiveYear.csv')

filtered_df = df

app = Dash(__name__)


app.layout = html.Div([
    dcc.Graph(id='graph-with-slider'),
    dcc.Slider(
        df['year'].min(),
        df['year'].max(),
        step=None,
        value=df['year'].min(),
        marks={str(year): str(year) for year in df['year'].unique()},
        id='year-slider'
    )
])


@callback(
    Output(component_id='graph-with-slider', component_property='figure'),
    Input(component_id='year-slider', component_property='value'))
def update_figure(selected_year):
    filtered_df = df[df.year == selected_year]

    fig = px.scatter(
        filtered_df, x="gdpPercap", y="lifeExp",
        size="pop", color="continent", hover_name="country",
        log_x=True, size_max=55
    )

    fig.update_layout(
        transition_duration=500,
        xaxis=dict(
            rangeslider=dict(
                visible=True
            )
        ),

    )
    return fig


if __name__ == '__main__':
    app.run(debug=False)
