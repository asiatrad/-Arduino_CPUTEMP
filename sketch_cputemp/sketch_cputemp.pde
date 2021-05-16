import processing.serial.*;

Serial serial;

void setup() {
size(120,120);
serial = new Serial(this, "/dev/cu.usbserial-1420", 9600);
}

void draw() {
delay(10000);

  StringList strout = new StringList(); //出力が入る変数
  StringList strerr = new StringList(); //エラーが入る変数

  shell(strout, strerr, "istats cpu --value-only & istats fan speed --value-only");

  println(strout);

  for(String el:strout){
     String a = trim(el);
    println(a); 
if( a.indexOf('.') != -1){ 
  int b = a.indexOf(".");
    String c = a.substring(0 , b);
    int high = new Integer(c);
    String e = a.substring(b + 1);
    int low = new Integer(e);
 
    serial.write('H');
    serial.write(high);
    serial.write(low);
  }
else{
    int value = new Integer(a);
    byte high = (byte)((value & 0xFF00) >> 8);
    byte low =  (byte)( value & 0x00FF);
    
  serial.write('R');  // ヘッダの送信
  serial.write(high); // 上位バイトの送信
  serial.write(low);  // 下位バイトの送信
  }  
 }
}
// データの受信
void serialEvent(Serial port) {
  if ( port.available() > 0 ) {
    String data = trim(port.readStringUntil('\n'));
    if ( data != null ) {
      println("data =", data);
    }
  }
}
