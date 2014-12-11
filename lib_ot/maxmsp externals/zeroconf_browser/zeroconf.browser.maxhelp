{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 917.0, 571.0, 640.0, 506.0 ],
		"bglocked" : 0,
		"defrect" : [ 917.0, 571.0, 640.0, 506.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 10.0,
		"default_fontface" : 0,
		"default_fontname" : "Verdana",
		"gridonopen" : 0,
		"gridsize" : [ 5.0, 5.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "browse _afpovertcp._tcp",
					"outlettype" : [ "" ],
					"fontsize" : 10.0,
					"patching_rect" : [ 51.0, 42.0, 135.0, 17.0 ],
					"id" : "obj-8",
					"numinlets" : 2,
					"fontname" : "Verdana",
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "browse _smb._tcp",
					"outlettype" : [ "" ],
					"fontsize" : 10.0,
					"patching_rect" : [ 82.0, 67.0, 103.0, 17.0 ],
					"id" : "obj-7",
					"numinlets" : 2,
					"fontname" : "Verdana",
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "zeroconf.service @name \"Open Sound Control UDP Service\" @port 4321 @type _osc._udp",
					"fontsize" : 10.0,
					"patching_rect" : [ 130.0, 318.0, 465.0, 19.0 ],
					"id" : "obj-5",
					"numinlets" : 1,
					"fontname" : "Verdana",
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "zeroconf.service @name \"Un autre service\" @port 6666 @type _osc._udp",
					"fontsize" : 10.0,
					"patching_rect" : [ 130.0, 344.0, 380.0, 19.0 ],
					"id" : "obj-6",
					"numinlets" : 1,
					"fontname" : "Verdana",
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "zeroconf.service @name \"Open Sound Control UDP Service\" @port 12345 @type _osc._udp",
					"fontsize" : 10.0,
					"patching_rect" : [ 130.0, 284.0, 471.0, 19.0 ],
					"id" : "obj-4",
					"numinlets" : 1,
					"fontname" : "Verdana",
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "browse _osc._udp",
					"outlettype" : [ "" ],
					"fontsize" : 10.0,
					"patching_rect" : [ 105.0, 99.0, 103.0, 17.0 ],
					"id" : "obj-3",
					"numinlets" : 2,
					"fontname" : "Verdana",
					"numoutlets" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print browser",
					"fontsize" : 10.0,
					"patching_rect" : [ 105.0, 178.0, 78.0, 19.0 ],
					"id" : "obj-2",
					"numinlets" : 1,
					"fontname" : "Verdana",
					"numoutlets" : 0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "zeroconf.browser @type _smb._tcp @domain local.",
					"outlettype" : [ "" ],
					"fontsize" : 10.0,
					"patching_rect" : [ 105.0, 125.0, 268.0, 19.0 ],
					"id" : "obj-1",
					"numinlets" : 1,
					"fontname" : "Verdana",
					"numoutlets" : 1
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-2", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
