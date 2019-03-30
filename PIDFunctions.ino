void calculatePID()
{
  float Kp = 0, Ki = 0, Kd = 0;
  static float pTerm, iTerm, dTerm;
  float zDot;
  float zDoubledot;
  if(initPID)
  {
    sincePrev = 0;
    initPID = false; 
  }
  /////////////////Looked at PID Library and uses sample rate of .1 sec
  // otherwise it was the fact that sincePrev was 0 that was giving nan
  if(sincePrev >= 10) 
  {
    pidError = pidDepth - targetDepth;
    if(pidError >= PID_RANGE && pidError <= HOLD_TOL) // leave as hold tol for now but may need to increase during testing
    {
      Kp = KP;
      Ki = KI;
      Kd = KD;
    } 
    else
    {
      Kp = KP2; /////////////Save as Constant /// .995/pidError
    }
    pTerm = Kp*pidError;
    iTerm += Ki*pidError;
    dTerm = Kd*((pidError-prevError)/sincePrev);
    zDot = (pidDepth - prevDepth)/(sincePrev);
    sincePrev = 0;
    zDoubledot = ((-RHO*A*CD*sq(zDot))/(2*M))+ g - (Fb/M) + (pTerm/M) + (iTerm/M) + (dTerm/M);
    thrust = -(M*zDoubledot);
    prevError = pidError;
    prevDepth = pidDepth;
  }
}


