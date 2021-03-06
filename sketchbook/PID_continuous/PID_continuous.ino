#include <PinChangeInt.h>

int pos = 11;
int neg = 10;

#define Xa A0
#define Xb A1

float kp = 0.2f;
float ki = 0.0000f;
float kd = 0.8f;

int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
uint8_t old_AB = 0;

int8_t read_encoder()
{
  int sensor1Value = digitalRead(Xa);
  int sensor2Value = digitalRead(Xb);
  uint8_t new_AB = (sensor1Value<<1)|sensor2Value;
  //uint8_t new_AB = ENC_PORT;

  old_AB <<= 2; //shift the values of pins 2 and 3 down to 0 and 1
  //so the rest of the logic works
  
  old_AB |= ( new_AB & 0x03 ); //add current state
  return ( enc_states[( old_AB & 0x0f )]);
}

int position = 0;
int lastPosition = 0;
int destination = 0;

void doEncoder()
{
  int8_t tmpdata;
  tmpdata = read_encoder();
  if( tmpdata )
  {
    position += tmpdata;
  }
}

void setup()
{
  pinMode(2, INPUT);     
  //pinMode(5, OUTPUT);     
  pinMode(3, INPUT);     
  //pinMode(7, OUTPUT);     
  pinMode(pos, OUTPUT);     
  pinMode(neg, OUTPUT);     

  //digitalWrite(4, HIGH);
  //digitalWrite(5, HIGH);
  //digitalWrite(6, HIGH);
  //digitalWrite(7, LOW);
  digitalWrite(pos, LOW);
  digitalWrite(neg, LOW);

  Serial.begin(9600);
  delay(1000);

#if 0
  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoder, CHANGE);
  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoder, CHANGE);
#else
  PCintPort::attachInterrupt(Xa,doEncoder,CHANGE);
  PCintPort::attachInterrupt(Xb,doEncoder,CHANGE);
#endif
}

static int state = 0;

void Speed( int v ) {
  if( v > 0 ) {
    digitalWrite(pos,LOW);
    analogWrite(neg,v);
  } else {
    v = -v;
    digitalWrite(neg,LOW);
    analogWrite(pos,v);
  }
}
void Speed2( float v ) {
	if( v > 1.0f )
		Speed( 255 );
	else if( v < -1.0f )
		Speed( -255 );
	else
		Speed( 255 * v );
}

void stop()
{
  digitalWrite( pos, LOW );
  digitalWrite( neg, LOW );
}

int newAngle = 0;
int frame = 0;
int comp = 0;
int I = 0;
void loop()
{
  byte inByte, zero = 0;
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    Serial.print("rec:");
    Serial.println(inByte, DEC);
    if( inByte == 'p' ) {
		 kp = newAngle / 100.0f;
		 newAngle = 0;
	 } else if( inByte == 'i' ) {
		 ki = newAngle / 10000.0f;
		 newAngle = 0;
	 } else if( inByte == 'd' ) {
		 kd = newAngle / 1000.0f;
		 newAngle = 0;
	 } else if( inByte == 'g' ) {
      destination = newAngle;
      newAngle = 0;
    } else {
      int dec = inByte-'0';
      newAngle *= 10;
      newAngle += dec;
    }
  }

  int dir = 0;
  int dist = dist = destination - position;
  int speed = position - lastPosition;
  I += dist;

  float PID = kp * dist + ki * I - kd * speed;
    
  if( destination > position )
  {
    dir = 1;
  }
  
  if( destination < position )
  {
    dir = -1;
    dist = -dist;
    speed = -speed;
  }
  
  Speed2( PID );

//  if( dist > 90 )
//  {
//    Speed(dir*255);
//  }
//  else
//  {
//    if( speed == 0 && dist > 10 )
//    {
//      comp += 1;
//      Speed(dir*(dist+comp));
//    }
//    else
//    {
//      comp = 0;
//      stop();
//    }
//  }

  
  lastPosition = position;
  ++frame;
  if( ( frame & 31 ) == 0 )
  {
    char buffer[ 24 ];
    sprintf( buffer, "%5i %4i  -> %5i : %5i %2i", position, speed, destination, dist, dir );
    Serial.println(buffer);
  }
  //Serial.println(old_AB, DEC);
  delay(5);
}


