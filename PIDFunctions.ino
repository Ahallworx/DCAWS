void calculatePID()   //For PID Basic only based on depth only.
{
  //PID constants
  Kp = KP;
  Ki = KI;
  Kd = KD;
  
  unsigned long currentTime, previousTime;
  double elapsedTime;
//(1)
//(1.1)
//(2)
//(2.1)
  currentTime = millis();                //get current time
  elapsedTime = (double)(currentTime - previousTime);   //compute time elapsed from previous compute
  pidError = pidDepth - targetDepth;     //Calculate error
//(3)
//(4)
//(4.1)
  pTerm = pidError;                         // get error
  iTerm += pidError*elapsedTime;           // compute integral
  dTerm = (pidError-prevError)/elapsedTime; // compute derivative
//(5)
    errorString += String(elapsedTime)+ "," + String(currentTime);                        
//(5.1)
//(6)
    thrust = (-Kp*pTerm - Ki*iTerm - Kd*dTerm);/*-(M*zDoubledot);*/
    prevError = pidError;
    previousTime = currentTime; 
    prevDepth = pidDepth;  
}








//(1)                         
//  float zDot;
//  float zDoubledot;
//(1.1)
//    if(initPID)
//    {
//      sincePrev = 0;
//      initPID = false; 
//    }
//(2)
//  /////////////////Looked at PID Library and uses sample rate of .1 sec
//  // otherwise it was the fact that sincePrev was 0 that was giving nan
//(2.1)
//    if(sincePrev >= 1000) 
//  {
//(3)
//    if(pidError >= PID_RANGE && pidError <= HOLD_TOL) // leave as hold tol for now but may need to increase during testing
//    {
//(4)
//    } 
//    else
//    {
//      Kp = KP2; /////////////Save as Constant /// .995/pidError
//    }
//(4.1)
//double sincePrevDterm = sincePrev; //truncate to double number.
//(5)
//zDot = (pidDepth - prevDepth)/(sincePrev);
//(5.1)
//sincePrev = 0;
//(6)
//zDoubledot = ((-RHO*A*CD*sq(zDot))/(2*M))+ g - (Fb/M) + (pTerm/M) + (iTerm/M) + (dTerm/M);

