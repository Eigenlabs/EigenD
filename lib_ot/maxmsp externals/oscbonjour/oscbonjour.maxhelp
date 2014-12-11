{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 943.0, 111.0, 634.0, 421.0 ],
		"bglocked" : 0,
		"defrect" : [ 943.0, 111.0, 634.0, 421.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 0,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "umenu",
					"numoutlets" : 3,
					"outlettype" : [ "int", "", "" ],
					"fontname" : "Arial",
					"types" : [  ],
					"items" : [ "Open Sound Control UDP Service", ",", "A long service name", ",", "AnotherOne" ],
					"fontsize" : 9.0,
					"id" : "obj-2",
					"numinlets" : 1,
					"patching_rect" : [ 98.0, 132.0, 116.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "2 browse for services",
					"numoutlets" : 0,
					"frgb" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontname" : "Arial",
					"textcolor" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontsize" : 14.0,
					"id" : "obj-3",
					"numinlets" : 1,
					"patching_rect" : [ 25.0, 70.0, 172.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "this object lists all the services of type \"_osc._udp\" registered through bonjour/zeroconf",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 10.0,
					"id" : "obj-4",
					"numinlets" : 1,
					"patching_rect" : [ 151.0, 26.0, 454.0, 18.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print resolved",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-5",
					"numinlets" : 1,
					"patching_rect" : [ 42.0, 214.0, 79.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print services",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-6",
					"numinlets" : 1,
					"patching_rect" : [ 82.0, 191.0, 79.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "4 test the connection",
					"numoutlets" : 0,
					"frgb" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontname" : "Arial",
					"textcolor" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontsize" : 14.0,
					"id" : "obj-7",
					"numinlets" : 1,
					"patching_rect" : [ 126.0, 266.0, 167.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "3 select destination",
					"numoutlets" : 0,
					"frgb" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontname" : "Arial",
					"textcolor" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontsize" : 14.0,
					"id" : "obj-8",
					"numinlets" : 1,
					"patching_rect" : [ 95.0, 155.0, 148.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "OscBonjour",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 24.0,
					"id" : "obj-9",
					"numinlets" : 1,
					"patching_rect" : [ 14.0, 10.0, 138.0, 34.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "/test zeroconf 1 2 3",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-10",
					"numinlets" : 2,
					"patching_rect" : [ 54.0, 292.0, 103.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend resolve",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-11",
					"numinlets" : 1,
					"patching_rect" : [ 79.0, 108.0, 89.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "OpenSoundControl service discovery and announcement using Bonjour for max/msp",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 10.0,
					"id" : "obj-12",
					"numinlets" : 1,
					"patching_rect" : [ 151.0, 11.0, 415.0, 18.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "udpsend localhost 9000",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-13",
					"numinlets" : 1,
					"patching_rect" : [ 26.0, 323.0, 120.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "browse",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-14",
					"numinlets" : 2,
					"patching_rect" : [ 26.0, 108.0, 51.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "oscbonjour",
					"numoutlets" : 3,
					"outlettype" : [ "", "", "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-15",
					"numinlets" : 1,
					"patching_rect" : [ 26.0, 132.0, 67.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "No need to know the IP and port anymore, just look for available OSC services",
					"linecount" : 2,
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-16",
					"numinlets" : 1,
					"patching_rect" : [ 329.0, 323.0, 268.0, 34.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print AnotherOne",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-17",
					"numinlets" : 1,
					"patching_rect" : [ 440.0, 284.0, 87.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "udpreceive 9999",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-18",
					"numinlets" : 1,
					"patching_rect" : [ 440.0, 258.0, 86.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print \"A long service name\"",
					"numoutlets" : 0,
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-19",
					"numinlets" : 1,
					"patching_rect" : [ 362.0, 178.0, 136.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "udpreceive 4000",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-20",
					"numinlets" : 1,
					"patching_rect" : [ 362.0, 152.0, 86.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "1 register the service",
					"numoutlets" : 0,
					"frgb" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontname" : "Arial",
					"textcolor" : [ 1.0, 0.360784, 0.682353, 1.0 ],
					"fontsize" : 14.0,
					"id" : "obj-21",
					"numinlets" : 1,
					"patching_rect" : [ 361.0, 69.0, 172.0, 23.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "it tells the Bonjour daemon that we are listening for OSC messages on the UDP port 4000",
					"linecount" : 5,
					"numoutlets" : 0,
					"frgb" : [ 0.8, 0.611765, 0.380392, 1.0 ],
					"fontname" : "Arial",
					"textcolor" : [ 0.8, 0.611765, 0.380392, 1.0 ],
					"fontsize" : 9.0,
					"id" : "obj-22",
					"numinlets" : 1,
					"patching_rect" : [ 457.0, 115.0, 101.0, 58.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "register AnotherOne 9999",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-23",
					"numinlets" : 2,
					"patching_rect" : [ 361.0, 230.0, 133.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "oscbonjour",
					"numoutlets" : 3,
					"outlettype" : [ "", "", "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-24",
					"numinlets" : 1,
					"patching_rect" : [ 361.0, 255.0, 67.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "register \"A long service name\" 4000",
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-25",
					"numinlets" : 2,
					"patching_rect" : [ 361.0, 95.0, 178.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "oscbonjour",
					"numoutlets" : 3,
					"outlettype" : [ "", "", "" ],
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-26",
					"numinlets" : 1,
					"patching_rect" : [ 361.0, 120.0, 67.0, 17.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-25", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 2 ],
					"destination" : [ "obj-2", 0 ],
					"hidden" : 0,
					"midpoints" : [ 83.5, 150.0, 95.0, 150.0, 95.0, 129.0, 107.5, 129.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 2 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-2", 1 ],
					"destination" : [ "obj-11", 0 ],
					"hidden" : 0,
					"midpoints" : [ 156.0, 152.0, 216.0, 152.0, 216.0, 103.0, 88.5, 103.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-13", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 0,
					"midpoints" : [ 88.5, 127.0, 35.5, 127.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 0 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 0 ],
					"destination" : [ "obj-13", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
