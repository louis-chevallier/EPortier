# sound



## schematic


```mermaid
flowgraph LR


subgraph ESP32
    d23
    d19
    d18
    d5
    _vin
    _GND
end
subgraph mcp3208
    vdd;
    gnd;
    clk;
    dout;
    din;
    cs;
end
d5 --> cs
d18 --> clk
d19 --> dout
d23 --> din
vdd --> _vin
gnd --> _GND

``` 
