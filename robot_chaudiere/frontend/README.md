## install systeme
a mettre dans cron ( via sudo crontab -u louis -e )

@reboot sleep 12 && cd /media/usb-seagate2/dev/git/EPortier/robot_chaudiere/frontend/ && make run >> /tmp/traceSensor.trc 2>&1


## schéma








xx



```mermaid
graph LR;
    subgraph arduino
        GND;
        P3V
        D1;
        D2;
        D3;
        A0;
    end
    subgraph DS18B20_Temp_Sensor
        Red;
        Yellow;
        Black;
    end
    subgraph MH_MQ2_Gaz_Sensor
        MH_A0 --- A0;
        MH_D0;
        MH_gnd --- GND;
        MH_vcc --- P3V;
        


    end
    subgraph DHT11_Temp_hygrometry_Sensor
        DHT11_VCC --- P3V;
        DHT11_Data --- D1;
        DHT11_nc ;
        DHT11_GND --- GND;
    end
    subgraph Relay
        relay_vcc --- GND;
        relay_in --- D3;
        relay_gnd --- GND;
    end

    Red --- P3V;
    Yellow --- D2
    Black --- GND;
    D2 --- |R1_3K23K| P3V;
    D1 --- |R2_4K5| P3V;



```













