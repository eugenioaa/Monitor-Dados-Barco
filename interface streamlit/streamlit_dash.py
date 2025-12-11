import streamlit as st
import pandas as pd
from sqlalchemy import create_engine, text
import plotly.express as px
from streamlit_autorefresh import st_autorefresh
import paho.mqtt.client as mqtt
from datetime import datetime, timedelta
import json
import base64

# config -----------------------------------------
st.set_page_config(layout = "wide", page_title = "monitoramento do barco")

#autorefresh a cada 2 s
st_autorefresh(interval=2000, key="datarefresh")

# customizando a cor das metricas -----------------------------------
st.markdown("""
    <style>
        div[data-testid="stMetricValue"] {
            color: #04a6b8 !important;
            font-size: 1.8rem;
        }
            </style>
""", unsafe_allow_html=True)

# mqtt ----------------------------------------
MQTT_BROKER = "mqtt.janks.dev.br"
TOPIC = "v3/projeto-iot/devices/44102/down/push"
MQTT_USER = "aula"
MQTT_PASSWORD = "zowmad-tavQez"

client = mqtt.Client()
client.username_pw_set(username=MQTT_USER, password=MQTT_PASSWORD)

client.connect(MQTT_BROKER, port=1883, keepalive=10)
client.loop_start()


# banco de dados ---------------------------------
DB_USER = "iot"
DB_PASSWORD = "pepcon-garton"
DB_HOST = "postgresql.janks.dev.br"
DB_PORT = "5432"
DB_NAME = "projeto"

# acessando banco 
@st.cache_resource
def get_db_engine():
    engine_str = f"postgresql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}:{DB_PORT}/{DB_NAME}"
    return create_engine(engine_str)

engine = get_db_engine()

# dados no dataframe
@st.cache_data(ttl=1) #recarrega dados do banco a cada meio segundo
def busca_dados():
    try:
        query = "SELECT * FROM dados_barco ORDER BY data_hora DESC LIMIT 50"
        df = pd.read_sql(query, engine)
        df['data_hora'] = pd.to_datetime(df['data_hora'] - timedelta(hours=3))
        return df
    except Exception as e:
        st.error(f"erro ao conectar: {e}")
        return pd.DataFrame()

df = busca_dados()

# sidebar --------------------------------------
col_esq, col_cent, col_dir = st.sidebar.columns([1,2,1])

with col_cent:
    st.sidebar.image("sharklogo.png", width=200)

st.sidebar.markdown("<h3 style='text-align: center; color: #04a6b8; font-size: 1.8rem;'>SHARK - STATUS</h3>", unsafe_allow_html=True)
st.sidebar.markdown("---")

st.sidebar.title("Leituras atuais")

latest_data = df.iloc[0]

st.sidebar.markdown(f"### **Última Leitura:** {latest_data['data_hora'].strftime('%H:%M:%S')}")
st.sidebar.markdown("---")

# botao de alerta ------------------------------------------

# inserir a mensagem de alerta no banco --------
def insere_banco(mensagem):
    with engine.connect() as conn:
        result = conn.execute(
            text("INSERT INTO alertas (data_hora, mensagem, status) VALUES (:data_hora, :msg, :status)"),
            {"data_hora": datetime.now(),"msg": str(mensagem),"status":'em aberto'}
        )
        conn.commit()


if "alerta_texto" not in st.session_state:
    st.session_state.alerta_texto = ""

with st.sidebar.form(key='alerta_form'):
    st.markdown("<h3 style='text-align: center; color: #F28F3B; font-size: 1.5rem;'>mensagem de alerta</h3>", unsafe_allow_html=True)
    # esse html xoxo aqui pra mudar coisas basicas no texto
    alerta_texto = st.text_area(
        "msg",
        label_visibility="collapsed",
        max_chars = 200,
        height = 100,
        placeholder = "digite sua mensagem"
    )
    
    botao_enviar = st.form_submit_button("enviar alerta")

    
    if botao_enviar:
            
            st.session_state.alerta_texto = alerta_texto

        # convertendo a string para base 64
            alerta_bytes = alerta_texto.encode("ascii")
            base64_bytes = base64.b64encode(alerta_bytes)
            base64_string = base64_bytes.decode("ascii")

            dicio = { 
                    "downlinks": [{ 
                    "f_port": 5, 
                    "frm_payload": base64_string, 
                    "priority": "HIGH" 
                    }] 
                    }
            
            try:
                client.publish(TOPIC, json.dumps(dicio))
                insere_banco(alerta_texto)
                print(alerta_texto)
                st.success("mensagem enviada via MQTT")
                
            except Exception as e:
                st.error(f"erro ao enviar: {e}")



# area das informacoes ------------------------------------------------

if not df.empty:
    
    col_mapa, col_graficos, col_metricas = st.columns([3, 2, 1])

    with col_mapa:
        st.markdown("### Localização")
        

        st.map(
            pd.DataFrame({'lat': [latest_data['latitude']], 'lon': [latest_data['longitude']]}),
            zoom=3, 
            width=500
        ) 
        
        st.divider()
        st.markdown(f"**Latitude:** {latest_data['latitude']:.4f} | **Longitude:** {latest_data['longitude']:.4f}")


    with col_graficos:
        
        # variacao de velocidade
        st.markdown("### Variação de velocidade")
        fig_vel = px.line(
            df.sort_values('data_hora'),
            x='data_hora', y='velocidade', 
            labels={'velocidade': 'nós', 'data_hora': 'hora'},
            color_discrete_sequence=["#3565CD"] 
        )
        fig_vel.update_traces(line=dict(width=3), mode="lines")
        fig_vel.update_layout(
            height=250, 
            margin=dict(l=20, r=20, t=10, b=10),
            xaxis=dict(showgrid=False),
            yaxis=dict(showgrid=True, gridcolor='rgba(128,128,128,0.2)'),
            hovermode="x unified"
        )
        st.plotly_chart(fig_vel)

        st.divider()

        # variação pressao no tanque
        st.markdown("### Pressão do tanque")
        fig_tanque = px.area(
            df.sort_values('data_hora'),
            x='data_hora', y='pressao_tanque', 
            labels={'pressao_tanque': 'bar', 'data_hora': 'hora'},
            color_discrete_sequence=["#28B463"]
        )
        fig_tanque.update_traces(line=dict(width=3))
        fig_tanque.update_layout(
            height=250,
            margin=dict(l=20, r=20, t=10, b=10),
            xaxis=dict(showgrid=False),
            yaxis=dict(showgrid=True, gridcolor='rgba(128,128,128,0.2)'),
            hovermode="x unified"
        )
        st.plotly_chart(fig_tanque)

    
    with col_metricas:
        st.markdown("### Resumo")
        
        with st.container(border=True):
            st.metric("Temp. Ar", f"{latest_data['temperatura_ar']:.1f} °C")
        
        with st.container(border=True):
            st.metric("Umidade", f"{latest_data['umidade']:.1f} %")
            
        with st.container(border=True):
            st.metric("Pressão Atm.", f"{latest_data['pressao_atmosferica']:.0f} hPa")
            
        with st.container(border=True):
            st.metric("Velocidade", f"{latest_data['velocidade']:.1f} nós")
            
        with st.container(border=True):
            direcoes = {1: "frente", 2: "esquerda", 3: "direita"}
            direcao_texto = direcoes.get(latest_data['direcao'], "desconhecida")
            st.metric("Direção", direcao_texto)


else:
    st.warning("Aguardando dados...")


