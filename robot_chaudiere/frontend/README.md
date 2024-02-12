## install systeme
a mettre dans cron ( via sudo crontab -u louis -e )

@reboot sleep 12 && cd /media/usb-seagate2/dev/git/EPortier/robot_chaudiere/frontend/ && make run >> /tmp/traceSensor.trc 2>&1


## schéma

```mermaid
graph LR;
    subgraph arduino
        GND;
        P3V
        D1;
        D2;
        A0;
    end
    subgraph Thermo1
        Red;
        Yellow;
        Black;
    end
    subgraph MH
        MHA0 --- A0;
        D0;
        plus --- P3V;
        gnd --- GND;
    end
    subgraph Thermo2
        P0 --- GND;
        P1;
        P2 --- D1;
        P3 --- GND;
    end

    Red --- P3V;
    Yellow --- D2
    Black --- GND;

    D2 -- R1_3_23K --- P3V;
    D1 -- R2_4_5K --- P3V;

```













