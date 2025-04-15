const int sensorPin=15;//15
int count_pre=0, count_cur=0 , pre=0, cur=0, diff=0;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  delay(1000);
  Serial.println("count:" + String(count_cur));
}

void loop() {
  cur=digitalRead(sensorPin);

  //if current is high and previous is low, increment by 1
  if (cur==1 and pre==0){
    count_cur++;
  }

  //to prevent same msg to print twice
  if(count_cur/2 != count_pre/2)
  {
    //dividing by 2 yield the best result for stepping
    Serial.println("count:" + String(count_cur/2));
    count_pre=count_cur;
  }

  //assign current to previous
  pre=cur;
  delay(50);
}

