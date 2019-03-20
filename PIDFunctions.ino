void calculatePID()
{
  float Kp = 0, Ki = 0, Kd = 0;
  float pTerm, iTerm, dTerm;
  float zDot;
  float zDoubledot;
  pidError = pidDepth - targetDepth;
  if(pidError >= PID_RANGE && pidError <= HOLD_TOL) // leave as hold tol for now but may need to increase during testing
  {
    Kp = KP;
    Ki = KI;
    Kd = KD;
  } 
  else
  {
    //Kp = 0;  //(CONST_THRUST - FBPOS - ((-RHO*A*CD*sq(zDot))/2)/pidError;
  }
  pTerm = Kp*pidError;
  iTerm += Ki*pidError;
  dTerm = Kd*((pidError-prevError)/sincePrev);
  zDot = (pidDepth - prevDepth)/(sincePrev);
  sincePrev = 0;
  zDoubledot = ((-RHO*A*CD*sq(zDot))/(2*M))+ g - (Fb/M) - (pTerm/M) - (iTerm/M) - (dTerm/M);
  thrust = -(M*zDoubledot);
}

