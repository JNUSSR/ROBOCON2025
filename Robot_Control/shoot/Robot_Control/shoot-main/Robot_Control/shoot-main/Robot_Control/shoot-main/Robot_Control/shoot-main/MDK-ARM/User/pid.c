float PID_calc(PID_typedef *PID, float measure, float target)
{
	if(PID == NULL)
		return 0;
	
	PID->measure += measure; // 累加测量值（可根据实际需求调整）
	PID->target = target;
	PID->error[2] = PID->error[1]; // 误差历史：2为上上次，1为上次
	PID->error[1] = PID->error[0]; // 误差历史：1为上次
	PID->error[0] = target - measure; // 当前误差=目标值-测量值
	
	if(fabsf(PID->error[0]) > PID->DeadBand || PID->DeadBand==0){ // 判断误差是否超出死区

		if(PID->mode == PID_POSITION_SPEED || PID->mode == PID_POSITION_ANGLE){ // 位置式PID
			
			if(PID->mode == PID_POSITION_ANGLE){ // 位置式PID-角度环
				if(PID->error[0]>4096) PID->error[0]=PID->error[0]-8191; // 角度误差处理，范围-4096~+4096
				else if(PID->error[0]<-4096) PID->error[0]=PID->error[0]+8191;
			}
			PID->Pout = PID->Kp * PID->error[0]; // 比例项
			PID->Iout += PID->Ki * PID->error[0]; // 积分项累加
			PID->D_item = (PID->error[0] - PID->error[1]); // 微分项（当前误差-上次误差）
			PID->Dout = PID->Kd * PID->D_item; // 微分输出
			LimitMax(PID->Iout,PID->Max_iout); // 积分限幅
			PID->OUT = PID->Pout + PID->Iout + PID->Dout; // PID输出
			LimitMax(PID->OUT,PID->Max_out); // 输出限幅
		}
		else if(PID->mode == PID_DELTA_SPEED){ // 增量式PID-速度环
			PID->Pout = PID->Kp * (PID->error[0] - PID->error[1]); // 比例项（误差增量）
			PID->Iout = PID->Ki * PID->error[0]; // 积分项
			PID->D_item = (PID->error[0] - 2.0f*PID->error[1] + PID->error[2]); // 微分项（本次-2*上次+上上次）
			PID->Dout = PID->Kd * PID->D_item; // 微分输出
			PID->OUT += PID->Pout + PID->Iout + PID->Dout; // PID输出累加
			LimitMax(PID->OUT, PID->Max_out); // 输出限幅
		}
	}
	else{
        PID->OUT=0; // 误差在死区内，输出为0
	}
	
	return PID->OUT; // 返回PID输出值
} 