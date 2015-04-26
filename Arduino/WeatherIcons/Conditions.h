const int NUM_IMAGES = 7;
char *filenames[NUM_IMAGES] = { "cloud.bmp", "lightnin.bmp", "rain.bmp", "snow.bmp", "sun.bmp", "suncloud.bmp", "wind.bmp" };
PImage images[NUM_IMAGES];
int conditions[] = {
//https:-1, //developer.yahoo.com/weather/documentation.html#codes
-1, //0	tornado
-1, //1	tropical storm
-1, //2	hurricane
1, //3	severe thunderstorms
1, //4	thunderstorms
-1, //5	mixed rain and snow
-1, //6	mixed rain and sleet
-1, //7	mixed snow and sleet
-1, //8	freezing drizzle
-1, //9	drizzle
-1, //10	freezing rain
2, //11	showers
2, //12	showers
3, //13	snow flurries
3, //14	light snow showers
-1, //15	blowing snow
3, //16	snow
-1, //17	hail
-1, //18	sleet
-1, //19	dust
-1, //20	foggy
-1, //21	haze
-1, //22	smoky
-1, //23	blustery
6, //24	windy
-1, //25	cold
0, //26	cloudy
5, //27	mostly cloudy (night)
5, //28	mostly cloudy (day)
5, //29	partly cloudy (night)
5, //30	partly cloudy (day)
-1, //31	clear (night)
4, //32	sunny
-1, //33	fair (night)
-1, //34	fair (day)
-1, //35	mixed rain and hail
-1, //36	hot
1, //37	isolated thunderstorms
1, //38	scattered thunderstorms
1, //39	scattered thunderstorms
1, //40	scattered showers
3, //41	heavy snow
3, //42	scattered snow showers
3, //43	heavy snow
5, //44	partly cloudy
1, //45	thundershowers
3, //46	snow showers
1, //47	isolated thundershowers
//3200	not available
};
