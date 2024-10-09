
#pragma once

#include "util.h"
namespace websocket {
  struct WSHandler *theWSH;
  struct WSHandler {
    WSHandler() { theWSH = this; }
    virtual void connect(AsyncWebSocket *) {};
    virtual void disconnect(AsyncWebSocket *) {};
    virtual void text(const String &) {};
    virtual void data(char *buf) {};
    virtual void binary(char *buf) {};
  };

  AsyncWebSocketClient * globalClient(NULL);;
  
  WSHandler *_theWSH = new WSHandler();
  char buff[3];

  void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
    //EKOX(type);
  
    if(type == WS_EVT_CONNECT){
      globalClient = client;
      EKOT("connect");
      EKOT("Websocket client connection received");
      theWSH->connect(server);
    } else if(type == WS_EVT_DISCONNECT){
      EKOT("disconnect");
      EKOT("Websocket client connection finished");
      globalClient = NULL;
    } else if(type == WS_EVT_DATA){
      //EKOX(len);
      AwsFrameInfo * info = (AwsFrameInfo*)arg;
      String msg = "";
      if(info->final && info->index == 0 && info->len == len){
        //the whole message is in a single frame and we got all of it's data
        //Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
        if(info->opcode == WS_TEXT){
          for(size_t i=0; i < info->len; i++) {
            msg += (char) data[i];         
          }
        } else {
          char buff[3];
          for(size_t i=0; i < info->len; i++) {
            sprintf(buff, "%02x ", (uint8_t) data[i]);
            msg += buff ;
          }
        }
        theWSH->text(msg);
      
        //EKOX(msg.c_str());
        auto t = split(msg);
        //EKOX(t.get<0>());
        //EKOX(t.get<1>());
      

        /*
          if(info->opcode == WS_TEXT)
          client->text("I got your text message");
          else
          client->binary("I got your binary message");
        */

      } else {
        EKO();
        //message is comprised of multiple frames or the frame is split into multiple packets
        if(info->index == 0){
          if(info->num == 0)
            Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
        }
        Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
        if(info->opcode == WS_TEXT){
          for(size_t i=0; i < len; i++) {
            msg += (char) data[i];
          }
        } else {
          for(size_t i=0; i < len; i++) {
            sprintf(buff, "%02x ", (uint8_t) data[i]);
            msg += buff ;
          }
        }
        Serial.printf("%s\n",msg.c_str());
        if((info->index + len) == info->len){
          Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
          if(info->final){
            Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
            if(info->message_opcode == WS_TEXT) {
              theWSH->text(msg);
              EKOT("I got your text message");
            } else {
              theWSH->binary(buff);
              EKOT("I got your binary message");
            }
          }
        }
      }
    }
  }
}
