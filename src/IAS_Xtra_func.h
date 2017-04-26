      volatile unsigned long org_buttonEntry;
      unsigned long org_buttonTime;
      volatile bool org_buttonChanged = false;
	  
	  
void ISRbuttonStateChanged() {  
  Serial.println("button changed...");
  if (digitalRead(MODEBUTTON) == 0){
    org_buttonEntry = millis();
  }else{
    if (millis() - org_buttonEntry >100){
    org_buttonTime = millis() - org_buttonEntry;
    org_buttonChanged = true;
    Serial.print("Time ");
    Serial.println(org_buttonTime);
    }
  }
}
void (*p)() = ISRbuttonStateChanged;