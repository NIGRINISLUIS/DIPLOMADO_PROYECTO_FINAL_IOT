import pandas as pd
import pika, os
from influxdb_client import InfluxDBClient, Point, WritePrecision
from datetime import timedelta
import datetime, time
from influxdb_client import InfluxDBClient, Point, Dialect
from influxdb_client.client.write_api import SYNCHRONOUS



import numpy as np
from sklearn import linear_model
from sklearn.metrics import mean_squared_error, r2_score
########################
from pandas.core.frame import DataFrame
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression
##from matplotlib import pyplot as plt

################################# CREDENCIALES A RABBIT E INFLUXDB EN PROXI########################
my_bucket = os.environ.get("DOCKER_INFLUXDB_INIT_BUCKET")
db_token = os.environ.get("DOCKER_INFLUXDB_INIT_ADMIN_TOKEN")
my_org = os.environ.get("DOCKER_INFLUXDB_INIT_ORG")
rabbit_user = os.environ.get("RABBIT_USER")
rabbit_password = os.environ.get("RABBIT_PASSWORD")
queue_name  = "mensajes"

############################## CREDENCIALES A RABBIT E INFLUXDB EN JUPYTER ###################################
#my_bucket = "DATOS_PROYECTO_FINAL"
#db_token = "test_token"
#my_org = "test_org"
#rabbit_user = "guest"
#rabbit_password = "guest"
#queue_name  = "mensajes"

############################ ACTIVACION DE ESCRITURA Y SOLICITUD DE DATOS EN INFLUXDB #######################
client = InfluxDBClient(url="http://20.119.217.173:8086", token=db_token, org=my_org)
write_api = client.write_api(write_options=SYNCHRONOUS)
query_api = client.query_api()


################################# FUNCION DE ENVIO Y SOLICITUD A INFLUXDB ########################################
def guardar_datos_influxdb_sin_procesar(a):
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




    print()
    print("##################### RECIBIENDO DATOS......#########################")
    print()
    write_api = client.write_api(write_options=SYNCHRONOUS)
    point = Point("DATOS FERMENTACION Y DESTILACION").field("FECHA", date_time).field("Horas", HF).field("Minutos", MF).field("Segundos", SF).field("Hora.", HD).field("Minutos.", MD).field("Segundos.", SD).field("TIEMPO_DESTILACION_{minutos}", TDM).field("TEMPERATURA", TM).field("MILILITROS_ALCOHOL", MA).field("PPM_ALCOHOL", PPM,)
    write_api.write(my_bucket, my_org, point)
    print()
    print("##################### ENVIANDO DATOS  A INFLUXDB---->")
    print()
    solicitar_datos_de_influxdb()
    return

def solicitar_datos_de_influxdb():
    print("######################### SOLICITAR DATOS ALMACENADOS......#####################")
    print()
    data_frame = query_api.query_data_frame('from(bucket:"DATOS_PROYECTO_FINAL") '
                                            '|> range(start: -15m) '
                                            '|> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value") '
                                            '|> keep(columns: ["TEMPERATURA", "MILILITROS_ALCOHOL", "PPM_ALCOHOL", "FECHA", "TIEMPO_DESTILACION_{minutos}"])')

    #datos_filtrados = data_frame.to_string()

    print("DATAFRAME DE DATOS SOLICITADOS")
    print()
    ##data_frame.set_index('FECHA', inplace = True)
    print(data_frame)
    print()
    print()
    print("ANALIZANDO DATOS....")
    print()
    print("#################### ANALISIS DE DATOS  ######################")
    print()

    print("Nuevo_df_influxDB ")
    print()
    Nuevo_df_influxDB = data_frame.loc[:,'MILILITROS_ALCOHOL':'TIEMPO_DESTILACION_{minutos}']
    print(Nuevo_df_influxDB )
    print()
    print("Veracidad de los datos")
    print()
    df_influxDB_procesado = Nuevo_df_influxDB.fillna(Nuevo_df_influxDB.mean())
    print()
    print("Dataframe para enviar a grafana")
    print(df_influxDB_procesado )
    print()
    
    print("ENVIO DE DATAFRAME REVISADO")
    point2 = Point("DATOS_DESTILACION_PROCESADOS").field("TEMPERATURA", float(df_influxDB_procesado['TEMPERATURA'].values[-1])).field("MILILITROS_ALCOHOL", float(df_influxDB_procesado['MILILITROS_ALCOHOL'].values[-1])).field("PPM_ALCOHOL", float(df_influxDB_procesado['PPM_ALCOHOL'].values[-1]))
    write_api.write("DESTILACION", my_org, point2)
    print()
       
    
    print("Veracidad de los datos a grafanas")
    df_veracidad_datos = df_influxDB_procesado.isnull().sum()
    print(df_veracidad_datos)
    print(df_veracidad_datos.MILILITROS_ALCOHOL)
    point2 = Point("VERACIDAD_DE_LOS_DATOS{null}").field("TEMPERATURA",df_veracidad_datos.TEMPERATURA).field("MILILITROS_ALCOHOL",df_veracidad_datos.MILILITROS_ALCOHOL).field("PPM_ALCOHOL",df_veracidad_datos.PPM_ALCOHOL)
    write_api.write("DESTILACION", my_org, point2)
    print()
    print()
    
    
    print("MAXIMO,MINIMO,MEDIA,DESVIACION")
    data_max = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].max()
    data_min = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].min()
    data_media = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].mean()
    data_std = df_influxDB_procesado.loc[:,'MILILITROS_ALCOHOL':'TEMPERATURA'].std()
    point3 = Point("MAXIMO,MINIMO,MEDIA,DESVIACION").field("TEMPERATURA_MAXIMA",data_max.TEMPERATURA).field("TEMPERATURA_MINIMA",data_min.TEMPERATURA).field("TEMPERATURA_MEDIA",data_media.TEMPERATURA).field("TEMPERATURA_DESVIACION_STANDAR",data_std.TEMPERATURA)
    write_api.write("DESTILACION", my_org, point3)
    
    data_resume = DataFrame([data_min, data_max, data_media, data_std], index=['min','max','mean', 'std'])
    print(data_resume)


    print("REGRESION LINEAL")
    
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
    
    point5 = Point("REGRESION LINEAL").field("Coefficients",coeficiente).field("Independent term",termino_independiente)\
    .field("Mean squared error",error_cuadratico_media).field("Variance score",varianza)
    write_api.write("DESTILACION", my_org, point5)
    print()
    #print(x,y)
    
    print()
    #print("prediccion")
    print()
    x_train = df_influxDB_procesado.iloc[:df_influxDB_procesado.shape[0]-1, [2, 3]]
    y_train = df_influxDB_procesado.iloc[:df_influxDB_procesado.shape[0]-1, 0]
    predict_value = DataFrame(df_influxDB_procesado[['TEMPERATURA','TIEMPO_DESTILACION_{minutos}']].values[-1], 
                              index=['TEMPERATURA', 'TIEMPO_DESTILACION_{minutos}'])
    #print(predict_value.shape ,type(predict_value))
    #print(predict_value)
    linear_reg = LinearRegression().fit(x_train, y_train)
    test_predict = linear_reg.predict(predict_value.T)
    #print("prediccion MILILITROS_ALCOHOL con respecto a las otras varibles ", test_predict)
    
    test_predict = float(test_predict)
    point6 = Point("REGRESION LINEAL").field("Prediccion_MILILITROS_ALCOHOL",test_predict)
    write_api.write("DESTILACION", my_org, point6)

    #client.close()
    return


###############################################  ACTUALIZACION DE DATOS ###################################

def process_function(msg):
  mesage = msg.decode("utf-8")
  #print(mesage)
  #enviar_DB(mesage)
  guardar_datos_influxdb_sin_procesar(mesage)
  return


while 1:

  url = os.environ.get('CLOUDAMQP_URL', 'amqp://{}:{}@20.119.217.173:5672/%2f'.format(rabbit_user, rabbit_password))
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
