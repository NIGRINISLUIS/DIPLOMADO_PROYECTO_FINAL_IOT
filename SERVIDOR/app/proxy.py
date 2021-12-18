import pandas as pd
import pika, os
from influxdb_client import InfluxDBClient, Point, WritePrecision
from datetime import timedelta
import datetime, time
from influxdb_client import InfluxDBClient, Point, Dialect
from influxdb_client.client.write_api import SYNCHRONOUS
import paho.mqtt.client as paho
import numpy as np
from sklearn import linear_model
from sklearn.metrics import mean_squared_error, r2_score
from pandas.core.frame import DataFrame
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression

################################# CREDENCIALES A RABBIT E INFLUXDB EN PROXI########################
my_bucket = os.environ.get("DOCKER_INFLUXDB_INIT_BUCKET")
db_token = os.environ.get("DOCKER_INFLUXDB_INIT_ADMIN_TOKEN")
my_org = os.environ.get("DOCKER_INFLUXDB_INIT_ORG")
rabbit_user = os.environ.get("RABBIT_USER")
rabbit_password = os.environ.get("RABBIT_PASSWORD")
queue_name  = "mensajes"

############################ ACTIVACION DE ESCRITURA ##########################################
client = InfluxDBClient(url="http://20.121.64.231:8086", token=db_token, org=my_org)
write_api = client.write_api(write_options=SYNCHRONOUS)
query_api = client.query_api()


#############################################################################################
def guardar_datos_influxdb_sin_procesar(a):
    ################################ Captura de datos  #######################################
    lista = []
    valores = a
    vector =  valores.split(",")
    now = datetime.datetime.now()
    now_local = now - timedelta(hours=5)
    date_time = now_local.strftime('%d/%m/%Y %H:%M:%S')
    HF =  int(vector[0])
    MF =  int(vector[1])
    SF =  int(vector[2])
    HD =  int(vector[3])
    MD =  int(vector[4])
    SD =  int(vector[5])
    TM =  float(vector[6])
    MA =  float(vector[7])
    PPM =  float(vector[8])
    TDM =  float(vector[9])
    ###################################### Envio de datos capturados a la base de datos InfluxDB ####################################
    write_api = client.write_api(write_options=SYNCHRONOUS)
    point = Point("DATOS FERMENTACION Y DESTILACION").field("FECHA", date_time).field("Horas", HF).field("Minutos", MF).field("Segundos", SF).field("Hora.", HD).field("Minutos.", MD).field("Segundos.", SD).field("TIEMPO_DESTILACION_{minutos}", TDM).field("TEMPERATURA", TM).field("MILILITROS_ALCOHOL", MA).field("PPM_ALCOHOL", PPM,)
    write_api.write(my_bucket, my_org, point)
    solicitar_datos_de_influxdb()
    return

def solicitar_datos_de_influxdb():
    ####################################################### Solicitar datos almacenados en InfluxDB #################################
    data_frame = query_api.query_data_frame('from(bucket:"DATOS_PROYECTO_FINAL") '
                                            '|> range(start: -2m) '
                                            '|> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value") '
                                            '|> keep(columns: ["TEMPERATURA", "MILILITROS_ALCOHOL", "PPM_ALCOHOL", "FECHA", "TIEMPO_DESTILACION_{minutos}"])')

    Nuevo_df_influxDB = data_frame.loc[:,'MILILITROS_ALCOHOL':'TIEMPO_DESTILACION_{minutos}']

    ############################# Promedio de los datos faltantes ####################################################
    df_influxDB_procesado = Nuevo_df_influxDB.fillna(Nuevo_df_influxDB.mean())

    ############################ Envio de datos completos a  influxDB ################################################
    point2 = Point("DATOS_DESTILACION_PROCESADOS").field("TEMPERATURA", float(df_influxDB_procesado['TEMPERATURA'].values[-1])).field("MILILITROS_ALCOHOL", float(df_influxDB_procesado['MILILITROS_ALCOHOL'].values[-1])).field("PPM_ALCOHOL", float(df_influxDB_procesado['PPM_ALCOHOL'].values[-1]))
    write_api.write("DESTILACION", my_org, point2)



    ############################# Veracidad de los datos  ############################################################
    df_veracidad_datos = df_influxDB_procesado.isnull().sum()

    ############################ Envio de datos filtrados a  influxDB #################################################
    point2 = Point("VERACIDAD_DE_LOS_DATOS{null}").field("TEMPERATURA",df_veracidad_datos.TEMPERATURA).field("MILILITROS_ALCOHOL",df_veracidad_datos.MILILITROS_ALCOHOL).field("PPM_ALCOHOL",df_veracidad_datos.PPM_ALCOHOL)
    write_api.write("DESTILACION", my_org, point2)


    ######################################### Estadistica Descriptiva #####################################################
    print("MAXIMO,MINIMO,MEDIA,DESVIACION")
    data_max = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].max()
    data_min = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].min()
    data_media = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].mean()
    data_std = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].std()
    point3 = Point("MAXIMO,MINIMO,MEDIA,DESVIACION").field("TEMPERATURA_MAXIMA",data_max.TEMPERATURA).field("TEMPERATURA_MINIMA",data_min.TEMPERATURA).field("TEMPERATURA_MEDIA",data_media.TEMPERATURA).field("TEMPERATURA_DESVIACION_STANDAR",data_std.TEMPERATURA)
    write_api.write("DESTILACION", my_org, point3)
    data_resume = DataFrame([data_min, data_max, data_media, data_std], index=['min','max','mean', 'std'])

    ###################################### Estadistica Predictiva ##########################################################
    dataX =df_influxDB_procesado[["TIEMPO_DESTILACION_{minutos}"]]
    X_train = np.array(dataX)
    y_train = df_influxDB_procesado['TEMPERATURA'].values
    # Creamos el objeto de Regresión Linear
    regr = linear_model.LinearRegression()
    # Entrenamos nuestro modelo
    regr.fit(X_train, y_train)
    # Hacemos las predicciones que en definitiva una línea (en este caso, al ser 2D)
    y_pred = regr.predict(X_train)
    # Veamos los coeficienetes obtenidos, En nuestro caso, serán la Tangente
    print('Coefficients: \n', regr.coef_)
    # Este es el valor donde corta el eje Y (en X=0)
    print('Independent term: \n', regr.intercept_)
    # Error Cuadrado Medio
    print("Mean squared error: %.2f" % mean_squared_error(y_train, y_pred))
    # Puntaje de Varianza. El mejor puntaje es un 1.0
    print('Variance score: %.2f' % r2_score(y_train, y_pred))
    coeficiente = float(regr.coef_)
    termino_independiente = float(regr.intercept_)
    error_cuadratico_media = float(mean_squared_error(y_train, y_pred))
    varianza = float(r2_score(y_train, y_pred))

    y = coeficiente*(1 + df_influxDB_procesado['TIEMPO_DESTILACION_{minutos}'].values[-1])+termino_independiente


    ############################################# Envio de datos de Analisis Predictivo con Respecto a la Temperatura ############################################
    point5 = Point("REGRESION LINEAL").field("Coefficients",coeficiente).field("Independent term",termino_independiente)\
    .field("Mean squared error",error_cuadratico_media).field("Variance score",varianza).field("prediccion_temperatura",y)\
    .field("temperatura_actual",df_influxDB_procesado['TEMPERATURA'].values[-1])
    write_api.write("DESTILACION", my_org, point5)

    ####################("REGRESION LINEAL PARA MILILITROS D3 ALCOHOL")

    dataX =df_influxDB_procesado[['TEMPERATURA']]
    X_train = np.array(dataX)
    y_train = df_influxDB_procesado['MILILITROS_ALCOHOL'].values
    # Creamos el objeto de Regresión Linear
    regr = linear_model.LinearRegression()
    # Entrenamos nuestro modelo
    regr.fit(X_train, y_train)
    # Hacemos las predicciones que en definitiva una línea (en este caso, al ser 2D)
    y_pred = regr.predict(X_train)
    # Veamos los coeficienetes obtenidos, En nuestro caso, serán la Tangente
    print('Coefficients: \n', regr.coef_)
    # Este es el valor donde corta el eje Y (en X=0)
    print('Independent term: \n', regr.intercept_)
    # Error Cuadrado Medio
    print("Mean squared error: %.2f" % mean_squared_error(y_train, y_pred))
    # Puntaje de Varianza. El mejor puntaje es un 1.0
    print('Variance score: %.2f' % r2_score(y_train, y_pred))
    coeficiente = float(regr.coef_)
    termino_independiente = float(regr.intercept_)
    error_cuadratico_media = float(mean_squared_error(y_train, y_pred))
    varianza = float(r2_score(y_train, y_pred))
    y = coeficiente*(1 + df_influxDB_procesado['TEMPERATURA'].values[-1])+termino_independiente

    ###################################### Envio de datos de Analisis predictivo ####################################3
    point6 = Point("REGRESION LINEAL MILILITROS").field("Coefficients",coeficiente).field("Independent term",termino_independiente)\
    .field("Mean squared error",error_cuadratico_media).field("Variance score",varianza).field("prediccion_mililitros",y)
    write_api.write("DESTILACION", my_org, point6)


    alerta_mqtt(y)
    return

################################################ envio de alerta  ########################################3333
def on_connect(client, userdata, flags, rc):
    client.subscribe('mensajes')
    client.publish('mensajes', 'activar_alarma', 0, False)

def on_publish(client, userdata, mid):
    print("mid: " + str(mid))

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

def alerta_mqtt(accion):
    mqtthost = "20.121.64.231"
    mqttuser = "guest"
    mqttpass = "guest"
    client = paho.Client()
    client.on_connect = on_connect
    client.on_publish = on_publish
    client.on_message = on_message
    client.username_pw_set(mqttuser, mqttpass)
    client.connect(mqtthost, 1883, 60)
    if(accion > 85):
        client.publish('mensajes', 'PRECAUCION', 0, False)

    return


###############################################  ACTUALIZACION DE DATOS ###################################


def process_function(msg):
  mesage = msg.decode("utf-8")
  #print(mesage)
  #enviar_DB(mesage)
  guardar_datos_influxdb_sin_procesar(mesage)
  return


while 1:

  url = os.environ.get('CLOUDAMQP_URL', 'amqp://{}:{}@20.121.64.231:5672/%2f'.format(rabbit_user, rabbit_password))
  params = pika.URLParameters(url)
  connection = pika.BlockingConnection(params)
  channel = connection.channel() # start a channel
  channel.queue_declare(queue=queue_name) # Declare a queue
  channel.queue_bind(exchange="amq.topic", queue=queue_name, routing_key='#')
  # create a function which is called on incoming messages
  def callback(ch, method, properties, body):
    process_function(body)

  # set up subscription on the queue
  channel.basic_consume(queue_name, callback, auto_ack=True)

  #start consuming (blocks)
  channel.start_consuming()
  connection.close()
################################################################################################################
