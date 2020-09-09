#pragma once

#define PT1 uint32_t	
#define PM1 systime		
#define SC1 0.0001		
#define DC1 "second"	

#define PT2 float	       
#define PM2 baro_pressure  
#define SC2 1	       
#define DC2 "millibar"     

#define PT3 float				
#define PM3 diff_pressure_central	
#define SC3 1				
#define DC3 "pascal"			
	                                    
#define PT4 float				
#define PM4 diff_pressure_horizontal	
#define SC4 1				
#define DC4 "pascal"			
	                                    
#define PT5 float			   	
#define PM5 diff_pressure_vertical 	
#define SC5 1				
#define DC5 "pascal"                     	

#define PT6  float				
#define PM6  air_velocity			
#define SC6  1				
#define DC6  "m/s"				
	                                    
#define PT7  float				
#define PM7  alpha_angle			
#define SC7  1				
#define DC7  "degree"			
	                                    
#define PT8  float				
#define PM8  beta_angle			
#define SC8  1				
#define DC8  "degree"			
	                                    
#define PT9  float				
#define PM9  accel_x			
#define SC9  1				
#define DC9  "m/s²" 			
	                                    
#define PT10 float				
#define PM10 accel_y			
#define SC10 1	    			
#define DC10 "m/s²" 			
	                                    
#define PT11 float				
#define PM11 accel_z			
#define SC11 1	    			
#define DC11 "m/s²" 			
	                                    
#define PT12 float				
#define PM12 gyro_x				
#define SC12 1				
#define DC12 "deg/s"			
	                                    
#define PT13 float				
#define PM13 gyro_y 			
#define SC13 1	    			
#define DC13 "deg/s"			
	                                    
#define PT14 float				
#define PM14 gyro_z  			
#define SC14 1	    			
#define DC14 "deg/s"                        

#define PT15  uint32_t		
#define PM15  rtc_time		
#define SC15  0.001			
#define DC15  "second"		
	                             
#define PT16  int32_t		
#define PM16  utm_east		
#define SC16  1			
#define DC16  "utm_coordinate"	
	                             
#define PT17  int32_t	     	
#define PM17  utm_north	     	
#define SC17  1		     	
#define DC17  "utm_coordinate"	
	   
#define PT18  int32_t		
#define PM18  altitude		
#define SC18  0.001			
#define DC18  "meters_above_geoid"	

#define PT19 int16_t						     
#define PM19 baro_temperature				     
#define SC19 0.01						     
#define DC19 "degree_celcius"				     
	                                                          
#define PT20 int16_t						     
#define PM20 diff_temperature_central			     
#define SC20 0.01						     
#define DC20 "degree_celcius"				     
	                                                          
#define PT21 int16_t						     
#define PM21 diff_temperature_horizontal		     
#define SC21 0.01						     
#define DC21 "degree_celcius"				     
	                                                          
#define PT22 int16_t						     
#define PM22 diff_temperature_vertical			     
#define SC22 0.01						     
#define DC22 "degree_celcius"				     
	                                                          
#define PT23 int16_t						     
#define PM23 attitude_x					     
#define SC23 0.01						     
#define DC23 "degree"  					     
	                                                          
#define PT24 int16_t   					     
#define PM24 attitude_y					     
#define SC24 0.01      					     
#define DC24 "degree"  					     
	                                                          
#define PT25 int16_t   					     
#define PM25 attitude_z					     
#define SC25 0.01      					     
#define DC25 "degree"  					     
	     						     
#define PT26 int16_t						     
#define PM26 course						     
#define SC26 0.1						     
#define DC26 "degree"					     
	                                                          
#define PT27 uint16_t					     
#define PM27 speed						     
#define SC27 0.01						     
#define DC27 "m/s"						     
	                                                          
#define PT28 int16_t						     
#define PM28 climb_speed					     
#define SC28 0.01						     
#define DC28 "m/s"						     
	                                                          
#define PT29 int16_t						     
#define PM29 ps_voltage						     
#define SC29 0.001						     
#define DC29 "volts"						     
	                                                          
#define PT30 int16_t						     
#define PM30 core_temperature				     
#define SC30 0.01						     
#define DC30 "degree_celcius"                                     
	     						     
#define PT31 uint8_t						     
#define PM31 utm_zone					     
#define SC31 1						     
#define DC31 "utm_zone"                                           


#define DECLS  \
    DECL(1);   \
    DECL(2);   \
    DECL(3);   \
    DECL(4);   \
    DECL(5);   \
    DECL(6);   \
    DECL(7);   \
    DECL(8);   \
    DECL(9);   \
    DECL(10);  \
    DECL(11);  \
    DECL(12);  \
    DECL(13);  \
    DECL(14);  \
    DECL(15);  \
    DECL(16);  \
    DECL(17);  \
    DECL(18);  \
    DECL(19);  \
    DECL(20);  \
    DECL(21);  \
    DECL(22);  \
    DECL(23);  \
    DECL(24);  \
    DECL(25);  \
    DECL(26);  \
    DECL(27);  \
    DECL(28);  \
    DECL(29);  \
    DECL(30);  \
    DECL(31);  

#define DESCS \
  DESC(1),    \
  DESC(2),    \
  DESC(3),    \
  DESC(4),    \
  DESC(5),    \
  DESC(6),    \
  DESC(7),    \
  DESC(8),    \
  DESC(9),    \
  DESC(10),   \
  DESC(11),   \
  DESC(12),   \
  DESC(13),   \
  DESC(14),   \
  DESC(15),   \
  DESC(16),   \
  DESC(17),   \
  DESC(18),   \
  DESC(19),   \
  DESC(20),   \
  DESC(21),   \
  DESC(22),   \
  DESC(23),   \
  DESC(24),   \
  DESC(25),   \
  DESC(26),   \
  DESC(27),   \
  DESC(28),   \
  DESC(29),   \
  DESC(30),   \
  DESC(31)
