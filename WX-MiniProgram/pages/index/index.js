//真机调试的话注释两处

import * as echarts from '../../ec-canvas/echarts';
const app = getApp();
var k=0
var dataarray = new Array(375);
var Chart = null
var dataList = [1,2,3,4,5]
var boardChannelDataInt = new Int32Array(5);

//阻抗检测
var ImpeIndex = 0;
var ImpeArray = new Array(256);
var ImpeResult = 0;
 
function cfft(amplitudes)
{
	var N = amplitudes.length;
	if( N <= 1 )
		return amplitudes;
 
	var hN = N / 2;
	var even = [];
	var odd = [];
	even.length = hN;
	odd.length = hN;
	for(var i = 0; i < hN; ++i)
	{
		even[i] = amplitudes[i*2];
		odd[i] = amplitudes[i*2+1];
	}
	even = cfft(even);
	odd = cfft(odd);
 
	var a = -2*Math.PI;
	for(var k = 0; k < hN; ++k)
	{
		if(!(even[k] instanceof Complex))
			even[k] = new Complex(even[k], 0);
		if(!(odd[k] instanceof Complex))
			odd[k] = new Complex(odd[k], 0);
		var p = k/N;
		var t = new Complex(0, a * p);
		t.cexp(t).mul(odd[k], t);
		amplitudes[k] = even[k].add(t, odd[k]);
		amplitudes[k + hN] = even[k].sub(t, even[k]);
	}
	return amplitudes;
}
function Complex(re, im) 
{
	this.re = re;
	this.im = im || 0.0;
}
Complex.prototype.add = function(other, dst)
{
	dst.re = this.re + other.re;
	dst.im = this.im + other.im;
	return dst;
}
Complex.prototype.sub = function(other, dst)
{
	dst.re = this.re - other.re;
	dst.im = this.im - other.im;
	return dst;
}
Complex.prototype.mul = function(other, dst)
{
	//cache re in case dst === this
	var r = this.re * other.re - this.im * other.im;
	dst.im = this.re * other.im + this.im * other.re;
	dst.re = r;
	return dst;
}
Complex.prototype.cexp = function(dst)
{
	var er = Math.exp(this.re);
	dst.re = er * Math.cos(this.im);
	dst.im = er * Math.sin(this.im);
	return dst;
}
Complex.prototype.log = function()
{
	/*
	although 'It's just a matter of separating out the real and imaginary parts of jw.' is not a helpful quote
	the actual formula I found here and the rest was just fiddling / testing and comparing with correct results.
	http://cboard.cprogramming.com/c-programming/89116-how-implement-complex-exponential-functions-c.html#post637921
	*/
	if( !this.re )
		console.log(this.im.toString()+'j');
	else if( this.im < 0 )
		console.log(this.re.toString()+this.im.toString()+'j');
	else
		console.log(this.re.toString()+'+'+this.im.toString()+'j');
}

//BandPass
//fs：125Hz
//corner:20-40Hz
//order:4
var Band_GAIN = 43.72500732853752;
var Band_xv = new Float32Array(9);//极点+1
var Band_yv = new Float32Array(9);//极点+1
function BandPass(temp)
{ Band_xv[0] = Band_xv[1]; Band_xv[1] = Band_xv[2]; Band_xv[2] = Band_xv[3]; Band_xv[3] = Band_xv[4]; Band_xv[4] = Band_xv[5]; Band_xv[5] = Band_xv[6]; Band_xv[6] = Band_xv[7]; Band_xv[7] = Band_xv[8]; 
  Band_xv[8] = temp / Band_GAIN;
  Band_yv[0] = Band_yv[1]; Band_yv[1] = Band_yv[2]; Band_yv[2] = Band_yv[3]; Band_yv[3] = Band_yv[4]; Band_yv[4] = Band_yv[5]; Band_yv[5] = Band_yv[6]; Band_yv[6] = Band_yv[7]; Band_yv[7] = Band_yv[8]; 
  Band_yv[8] =   (Band_xv[0] + Band_xv[8]) - 4 * (Band_xv[2] + Band_xv[6]) + 6 * Band_xv[4]
               + ( -0.06321169571625 * Band_yv[0]) + (  0.04735712836140 * Band_yv[1])
               + ( -0.42206816081015 * Band_yv[2]) + (  0.25050862771226 * Band_yv[3])
               + ( -1.17396364229346 * Band_yv[4]) + (  0.46842762108394 * Band_yv[5])
               + ( -1.47030184363278 * Band_yv[6]) + (  0.38778804665416 * Band_yv[7]);
  return Band_yv[8];
}

//HighPass
//fs：125Hz
//corner:0.5Hz
//order:4
var High_GAIN = 1.03338365651397;
var High_xv = new Float32Array(5);//极点+1
var High_yv = new Float32Array(5);//极点+1
function HighPass(temp)  
{ High_xv[0] = High_xv[1]; High_xv[1] = High_xv[2]; High_xv[2] = High_xv[3]; High_xv[3] = High_xv[4]; 
  High_xv[4] = temp / High_GAIN;
  High_yv[0] = High_yv[1]; High_yv[1] = High_yv[2]; High_yv[2] = High_yv[3]; High_yv[3] = High_yv[4]; 
  High_yv[4] =   (High_xv[0] + High_xv[4]) - 4 * (High_xv[1] + High_xv[3]) + 6 * High_xv[2]
               + ( -0.93643324315202 * High_yv[0]) + (  3.80723245722885 * High_yv[1])
               + ( -5.80512542105514 * High_yv[2]) + (  3.93432582079874 * High_yv[3]);
  return High_yv[4];
}

//LowPass
//fs：125Hz
//corner:18Hz
//order:4
var Low_GAIN = 61.51886428008226;
var Low_xv = new Float32Array(5);//极点+1
var Low_yv = new Float32Array(5);//极点+1
function LowPass(temp) 
{ Low_xv[0] = Low_xv[1]; Low_xv[1] = Low_xv[2]; Low_xv[2] = Low_xv[3]; Low_xv[3] = Low_xv[4]; 
  Low_xv[4] = temp / Low_GAIN;
  Low_yv[0] = Low_yv[1]; Low_yv[1] = Low_yv[2]; Low_yv[2] = Low_yv[3]; Low_yv[3] = Low_yv[4]; 
  Low_yv[4] =   (Low_xv[0] + Low_xv[4]) + 4 * (Low_xv[1] + Low_xv[3]) + 6 * Low_xv[2]
               + ( -0.08515392332606 * Low_yv[0]) + (  0.53516628135463 * Low_yv[1])
               + ( -1.37575435011780 * Low_yv[2]) + (  1.66565916729657 * Low_yv[3]);
  return Low_yv[4];
}

//Notch
//fs：125Hz
//corner:49-51Hz
//order:2
var Notch_GAIN = 1.07367693745520;
var Notch_xv = new Float32Array(5);//极点+1
var Notch_yv = new Float32Array(5);//极点+1
function NotchFilter(temp)
{ Notch_xv[0] = Notch_xv[1]; Notch_xv[1] = Notch_xv[2]; Notch_xv[2] = Notch_xv[3]; Notch_xv[3] = Notch_xv[4]; 
  Notch_xv[4] = temp / Notch_GAIN;
  Notch_yv[0] = Notch_yv[1]; Notch_yv[1] = Notch_yv[2]; Notch_yv[2] = Notch_yv[3]; Notch_yv[3] = Notch_yv[4]; 
  Notch_yv[4] =   (Notch_xv[0] + Notch_xv[4]) +   3.24016044073991 * (Notch_xv[1] + Notch_xv[3]) +   4.62465992043396 * Notch_xv[2]
               + ( -0.86747213379167 * Notch_yv[0]) + ( -2.91046404408562 * Notch_yv[1])
               + ( -4.30259605835520 * Notch_yv[2]) + ( -3.12516981877757 * Notch_yv[3]);
   return Notch_yv[4];
} 

//Notch:20-40Hz
//fs：125Hz
//corner:20-40Hz
//order:2
var Notch31_GAIN = 2.07970066271631;
var Notch31_xv = new Float32Array(5);//极点+1
var Notch31_yv = new Float32Array(5);//极点+1
function Notch31(temp)
{ Notch31_xv[0] = Notch31_xv[1]; Notch31_xv[1] = Notch31_xv[2]; Notch31_xv[2] = Notch31_xv[3]; Notch31_xv[3] = Notch31_xv[4]; 
  Notch31_xv[4] = temp / Notch31_GAIN;
  Notch31_yv[0] = Notch31_yv[1]; Notch31_yv[1] = Notch31_yv[2]; Notch31_yv[2] = Notch31_yv[3]; Notch31_yv[3] = Notch31_yv[4]; 
  Notch31_yv[4] =   (Notch31_xv[0] + Notch31_xv[4]) -   0.28661435983197 * (Notch31_xv[1] + Notch31_xv[3]) +   2.02053694781547 * Notch31_xv[2]
               + ( -0.25232462628227 * Notch31_yv[0]) + (  0.08424157393608 * Notch31_yv[1])
               + ( -0.68090404450357 * Notch31_yv[2]) + (  0.19138882323620 * Notch31_yv[3]);
  return Notch31_yv[4];
}

var timer30;
var TimeOpen=1;//定时器先允许建立

function countdown(that) {
  var second = that.data.second
  if (second == 0) {
  //  that.setData({
  //   second: "采集完成"
  //  });
   wx.navigateTo({
    url: '../AI-report/AIreport'
  })
   return ;
  }
  timer30 = setTimeout(function(){
   that.setData({
    second: second - 1
   });
   countdown(that);
  }
  ,1000)
 }

Page({
  data: {
    receiveText: '',
    name: '',
    connectedDeviceId: '',
    services: {},
    characteristics: {},
    connected: true,
    IfHide:"block",
    second: 7,
    ec4: {
      onInit: initChart4
    }, 
  },


  AIreport:function (event) {
    console.log(event)
    wx.navigateTo({
      url: '../AI-report/AIreport'
    })
  },
  ReturnToConnection:function(){
    var that=this
    wx.closeBLEConnection({
      deviceId: that.data.connectedDeviceId
    })
    wx.navigateTo({
      url: '../search/search'
    })
  },
  
  onLoad: function (options) {
    var that = this
    var index=1
    
    console.log(options)
    that.setData({
      name: options.name,
      connectedDeviceId: options.connectedDeviceId
    })
    // setInterval(function(){
    //   index++
    //   dataList.shift()
    //   dataList.push(index)
    //   Chart.setOption({
    //     series:[{
    //       data:dataList
    //     }]
    //   })
    // },1000)
    wx.getBLEDeviceServices({
      deviceId: that.data.connectedDeviceId,
    
      success: function (res) {
        console.log(res.services)
        that.setData({
          services: res.services
        })
        wx.getBLEDeviceCharacteristics({
          deviceId: options.connectedDeviceId,
          serviceId: res.services[0].uuid,
          success: function (res) {
            console.log(res.characteristics)
            that.setData({
              characteristics: res.characteristics
            })
            wx.notifyBLECharacteristicValueChange({
              state: true,
              deviceId: options.connectedDeviceId,
              serviceId: that.data.services[0].uuid,
              characteristicId: that.data.characteristics[1].uuid,
              success: function (res) {
                console.log('启用notify成功')
                console.log(index)
              },
              complete:function(res){
                wx.onBLECharacteristicValueChange(function (res) {
                  var recText = new DataView(res.value)
                  // console.log('data',recText.getUint8())
                  for(var i=0;i<5;i++) {
                    for(var j=0;j<3;j++) {
                      boardChannelDataInt[i] = (boardChannelDataInt[i] << 8) | recText.getUint8(i*3+j)
                    }      
                    // console.log('boardChannelDataInt',boardChannelDataInt[i])             
                  }
                  for (var i = 0; i < 5; i++)
                  { //convert 3 byte 2's compliment to 4 byte 2's compliment
                    if ((boardChannelDataInt[i] & 0x00800000) == 0x00800000)
                    {
                      boardChannelDataInt[i] |= 0xFF000000;
                    }
                    else
                    {
                      boardChannelDataInt[i] &= 0x00FFFFFF;
                    }

                    ImpeArray.shift();
                    ImpeArray.push(BandPass(boardChannelDataInt[i]));
                    dataarray.shift();
                    dataarray.push(HighPass(LowPass(Notch31(NotchFilter(boardChannelDataInt[i])))));
                      
                  }

                  /*阻抗计算*/
                  ImpeIndex++;
                  if(ImpeIndex == 52) {//52x5=260,256计算一次FFT
                    ImpeIndex = 0;
                    cfft(ImpeArray);
                    // console.log('阻抗值',(ImpeArray[64]));
                    // console.log('阻抗值',Math.abs(ImpeArray[64].re));
                    ImpeResult = Math.abs(ImpeArray[64].re);
                  }

                  if(ImpeResult < 8000000) {
                    console.log('TimeOpen',TimeOpen)
                    if(TimeOpen == 1) {
                      TimeOpen = 0;
                      countdown(that);
                    }
                    that.setData({ 
                      IfHide: "none" ,
                    })
                    Chart.setOption({
                      series:[{
                        data:dataarray
                      }]
                    })
                  }
                  else {
                    clearTimeout(timer30);
                    TimeOpen = 1;
                    that.setData({ 
                      IfHide: "block" ,
                      second: 7
                    })
                    Chart.setOption({
                      series:[{
                        data:0
                      }]
                    })
                  }



                })

              }
            })
          }
        })
      }
      
    })
    wx.onBLEConnectionStateChange(function (res) {
      if(!res.connected){
        wx.showToast({
          title: '连接已断开',
          icon: 'error',
          duration: 1000
        })
        wx.navigateTo({
          url: '../search/search'
        })
      }
    })
  },

});

function initChart4(canvas, width, height, dpr) {
  Chart = echarts.init(canvas, null, {
    width: width,
    height: height,
    devicePixelRatio: dpr // 像素
  });
  canvas.setChart(Chart);

  var option = {
    tooltip: {
      trigger: 'axis',
      axisPointer: {
        animation: false
      }
    },
    legend: {
      top: "0%",
      right: '10%',
      textStyle: {
        color: "rgba(255,255,255,.5)",
        fontSize: "12"
      }
    },
    grid: {
      top: '10%',
      left: '1%',
      right: '1%',
      bottom: '10%',
      containLabel: true
    },
    xAxis: [{
      type: 'category',
      name: '时间',
      min:0,
      max:375,
      splitNumber: 11,
      interval:'10',
      nameLocation:'center',
      nameGap:25,
      boundaryGap: true,
      // 文本颜色为rgba(255,255,255,.8)  文字大小为 12
      axisLabel: {
        textStyle: {
          color: "rgba(0,0,0,.8)",
          fontSize: 10
        }
      },
     
      // x轴线的颜色为   rgba(255,255,255,.2)
      axisLine: {
        lineStyle: {
          color: "rgba(0,0,0,.2)"
        }
      },
      splitLine: {
        show: false
      },
      data: Array.from(new Array(375).keys()).slice(0)
    }],
    yAxis: [{
      show:false,
      type: 'value',
      min:-10000,
      max:20000,
      splitNumber: 7,
      scale: true,
      axisLine: {
        lineStyle: {
          color: "rgba(0,0,0,.2)"
        }
      },
      axisLabel: {
        show:false,
        textStyle: {
          color: "rgba(0,0,0,.8)",
          fontSize: 12
        }
      },
      splitLine: {
        show: false
      },
   
      
    }],
    series: [{
        type: 'line',
        lineStyle: {
          color: "#FF0000",
          width: 5
        },
        animation:false,
        showSymbol: false,

        data: dataarray
      }]
  };

  Chart.setOption(option);
  return Chart;
}
