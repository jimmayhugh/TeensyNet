int x;

void setup(void)
{
  for(x=0; x<34; x++)
  {
    pinMode(x, OUTPUT);
  }  
}

void loop(void)
{
  for(x=0; x<34; x++)
  {
    digitalWrite(x, HIGH);
  }
  delay(1000);
  
  for(x=0; x<34; x++)
  {
    digitalWrite(x, LOW);  
  }
  
  delay(1000);
}
