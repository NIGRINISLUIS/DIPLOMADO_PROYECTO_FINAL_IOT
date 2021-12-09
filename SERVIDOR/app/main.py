from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import paho.mqtt.client as mqtt
import time
import os
import pika

app = FastAPI(title="api de botones", description="esta es una api de control", version="1.0.0")

origins = [
    "http://localhost",
    "http://localhost:8080",
    "http://localhost:3000",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/")
async def index():
      connection = pika.BlockingConnection(pika.ConnectionParameters("20.119.217.173"))
      channel = connection.channel()
      channel.queue_declare(queue='mensajes')
      channel.basic_publish(exchange='amq.topic',
                      routing_key='mensajes',
                      body='MENSAJE HACIA  LA APLICACION MQTIZER')
print("'MENSAJE EN LA TERMINAL'")
