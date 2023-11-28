import plotly.graph_objects as go

import pandas as pd

# Load data
df = pd.read_csv(
    "/Users/bernhardklein/Library/Mobile Documents/com~apple~CloudDocs/FPV/POD_Racer/PODRacer/python/data/podrdata.csv")
#df.columns = [col.replace("AAPL.", "") for col in df.columns]

# Create figure
fig = go.Figure()

fig.add_trace(
#    go.Scatter(x=list(df.Date), y=list(df.High)))
    go.Scatter(x=list(d.TIME), y=list(df.CH_H)))

# Set title
fig.update_layout(
    title_text="Time series with range slider and selectors"
)

# Add range slider
fig.update_layout(
    xaxis=dict(
        rangeselector=dict(
            buttons=list([
                dict(count=1,
                     label="1m",
                     step="month",
                     stepmode="backward"),
                dict(count=6,
                     label="6m",
                     step="month",
                     stepmode="backward"),
                dict(count=1,
                     label="YTD",
                     step="year",
                     stepmode="todate"),
                dict(count=1,
                     label="1y",
                     step="year",
                     stepmode="backward"),
                dict(step="all")
            ])
        ),
        rangeslider=dict(
            visible=True
        ),
        type="date"
    )
)

fig.show()