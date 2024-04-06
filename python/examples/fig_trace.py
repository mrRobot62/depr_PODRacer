import pandas as pd
import plotly.express as px
import plotly.graph_objects as go

df = pd.read_csv("2018_eng.csv")

cols = df.columns[2:4].values.tolist() # "GDP per Capita" & "Social Support"

fig = go.Figure()
for col in cols:
    figpx = px.scatter(df.assign(Plot=col),
                       x=col,
                       y="Score",
                       size="Population",
                       color="Continent",
                       hover_name="Country/Region",
                       hover_data=["Plot"],
                       size_max=60,
                       color_discrete_sequence=px.colors.qualitative.G10).update_traces(visible=False)
    
    fig.add_traces(figpx.data)

fig.update_layout(
    updatemenus=[
        {
            "buttons": 
            [
                {
                    "label": k,
                    "method": "update",
                    "args": 
                    [
                        {"visible": [t.customdata[0][0]==k for t in fig.data]},
                    ],
                }
                for k in cols
            ]
        }
    ]
).update_traces(visible=True, selector=lambda t: t.customdata[0][0]==cols[0] )

fig