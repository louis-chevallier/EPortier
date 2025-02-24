## install systeme
a mettre dans cron ( via sudo crontab -u louis -e )

@reboot sleep 12 && cd /media/usb-seagate2/dev/git/EPortier/robot_chaudiere/frontend/ && make run >> /tmp/traceSensor.trc 2>&1


## schéma








xx


![DHT11](https://components101.com/sites/default/files/component_pin/DHT11%E2%80%93Temperature-Sensor-Pinout.jpg)










```mermaid
graph TD;


    subgraph arduino
        GND;
        P3V
        D1;

        D3;
        D5;
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
    Yellow --- D5;
    Black --- GND;
    D5 --- |R1_2K2_RRR| P3V;
    D1 --- |R2_3K3_OOR| P3V;




```

## Réseau

un nodemcu s'allume
il se connecte au reseau wifi de ssid/pw connu ( car je peux toujours donner le ssid que je veux )











