void getDepth()
{
  double sensorValue;
  double sensorVoltage_3V;
  double sensorVoltage_5V;
  double scaledVoltage;
  double pressure_psi;
  double pressure_Pa;
  sensorValue = analogRead(DEPTH_SENSOR);
  sensorVoltage_3V = sensorValue*(Vt/1024);
  sensorVoltage_5V = sensorVoltage_3V*(Vs/Vt);
  scaledVoltage = (sensorVoltage_5V*((.8*Vs)/Vs))+ (.1*Vs);
  pressure_psi = scaledVoltage*(100/(.8*Vs));
  pressure_Pa = pressure_psi*6894.76; //convert psi to pascal
  depth = pressure_Pa/(RHO*g);
}

void setTargetDepth(int count)
{
  if(count == 1)
    targetDepth = targetDepth_1;
  if(count == 2)
    targetDepth = targetDepth_2;
  if(count == 3)
    targetDepth = targetDepth_3;  
}

void logDepth()
{
  File DCAWS_Depth = SD.open("DCAWS_Depth.csv", FILE_WRITE);  
  DCAWS_Depth.println(depth);
  DCAWS_Depth.close(); 
}

// 64-bit double-precision moving average
// inputs:  latest depth (double), init (boolean)
// output:  depth moving average (double)
void mvavg_depth(double z,boolean mvinit, double zmean)
{
  // These static variables are kept in memory at all time
  // and can only be used by this function
  static double zsum;    // cumulated depth
  static int n;   // actual window size
  static int k;   // circular buffer index

  // if this is the first call, cumulated z value = input z value
  // and window size = 1; buffer the values
  if (mvinit == true)
  {
    zsum = z;
    n = 1;
    k = 0;
    zbuffer[k] = z; 
  }
  // if this is not the first call
  else
  {
    // if we have collected less points than the window size
    // cumulate the value and increment the window size
    if (n < winsz)
    {
      zsum += z;
      zbuffer[++k] = z;
      n++;
    }
    // if we have already collected more points than the window
    // size, add the latest record and remove the oldest one
    // we also replace the oldest record in the circular buffer
    // with the new one
    else
    {
      k++;
      k %= winsz;
      zsum += (z - zbuffer[k]);
      zbuffer[k] = z;
    }
  }
  
  // Calculate and return the moving average
  zmean = zsum / (double)n;
}
