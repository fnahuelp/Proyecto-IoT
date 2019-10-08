
#define STASSID "ox-demo"
#define STAPSK "ox-over-dns"

#define INFLUXDB_HOST "influxdb-c.catedras.linti.unlp.edu.ar"
#define INFLUXDB_PORT 8086

#define INFLUXDB_DATA "guillermocalderaro"
#define INFLUXDB_USER "guillermocalderaro"
#define INFLUXDB_PASS "guillermocalderaro_iot"

#define INTDELAY 30
#define RDELAY 100

const byte pinExt = D5;
const byte pinInt = D7;
volatile bool intExt = false;
volatile bool intInt = false;
bool exterior = 0;
bool interior = 0;
int loopCount = 0;
int dtimeExt = 0;
int dtimeInt = 0;

Influxdb influx(INFLUXDB_HOST, INFLUXDB_PORT);

void pinIntExt() {
  intExt = true;
}

void pinIntInt() {
  intInt = true;
}

void ResetState (){
  Serial.print("Reset");
  intExt = false;
  intInt = false;
  exterior = false;
  interior = false;
  loopCount = 0;
}

void SendInflux (int c){
  Serial.print("Mandando... ");
  InfluxData row("ingreso");
  row.addValue("valor", c);
  influx.write(row);
  Serial.print("Enviado: ");
  Serial.println(c);
}

inline void SendReset (int c){
  SendInflux(c);
  ResetState();
}

bool IntStateInt (){
  if (intInt){
    intInt = false;
    if (dtimeInt == 0){
      dtimeInt = INTDELAY;
      return true;
    }
  }
  return false;
}

bool IntStateExt (){
  if (intExt){
    intExt = false;
    if (dtimeExt == 0){
      dtimeExt = INTDELAY;
      return true;
    }
  }
  return false;
}

void setup() {
  
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  influx.setDbAuth(INFLUXDB_DATA, INFLUXDB_USER, INFLUXDB_PASS);

  pinMode(pinExt, INPUT_PULLUP);
  pinMode(pinInt, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinExt), pinIntExt, RISING);
  attachInterrupt(digitalPinToInterrupt(pinInt), pinIntInt, RISING);

}

void loop() {
  if (IntStateExt()){
    Serial.println("Exterior");
    if (interior)
    SendReset(-1);
    else exterior = true;
  }
  else if (IntStateInt()){
    Serial.println("Interior");
    if (exterior)
    SendReset(1);
    else interior = true;
  }
  else if (interior || exterior)
  if (++loopCount > RDELAY) SendReset(0);
  if (dtimeInt) --dtimeInt;
  if (dtimeExt) --dtimeExt;
  delay (50);
}
