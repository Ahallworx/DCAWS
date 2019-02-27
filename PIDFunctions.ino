void calculatePID()
{
  PID_error = targetDepth - avgdepth/*pidDepth*/;
  if(PID_error /*>*/<= PID_RANGE && PID_error <= HOLD_TOL) // again not sure on negatives
  {
    Kp = 2.6;
    Ki = .225;
    Kd = 10;
  } 
//  else
//  {
//    Kp = (CONST_THRUST - FBPOS - ((-RHO*A*CD*sq(z_dot))/2)/PID_ERROR;
//    Ki = 0;
//    Kd = 0;
//   }
  PID_p = Kp*PID_error;
  PID_i += Ki*PID_error;
  PID_d = Kd*((PID_error-prev_error)/sincePrev);
  z_dot = (avgdepth -prev_depth)/(sincePrev);
  sincePrev = 0;
  z_doubledot = ((-RHO*A*CD*sq(z_dot))/(2*M))+ g - (Fb/M) - (PID_p/M) - (PID_i/M) - (PID_d/M);
  thrust = -(M*z_doubledot);//does this need (-) or am I sleep deprived
}

