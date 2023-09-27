<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="SamacSys_Parts">
<description>&lt;b&gt;https://eagle.componentsearchengine.com&lt;/b&gt;&lt;p&gt;&lt;author&gt;Created by SamacSys&lt;/author&gt;</description>
<packages>
<package name="1381">
<description>&lt;b&gt;1381-3&lt;/b&gt;&lt;br&gt;
</description>
<pad name="1" x="0" y="0" drill="1.11" diameter="1.665" shape="square"/>
<pad name="2" x="0" y="-2.54" drill="1.11" diameter="1.665"/>
<pad name="3" x="0" y="-5.08" drill="1.11" diameter="1.665"/>
<pad name="4" x="0" y="-7.62" drill="1.11" diameter="1.665"/>
<pad name="5" x="0" y="-10.16" drill="1.11" diameter="1.665"/>
<pad name="6" x="0" y="-12.7" drill="1.11" diameter="1.665"/>
<pad name="7" x="0" y="-15.24" drill="1.11" diameter="1.665"/>
<pad name="8" x="0" y="-17.78" drill="1.11" diameter="1.665"/>
<pad name="9" x="0" y="-20.32" drill="1.11" diameter="1.665"/>
<pad name="10" x="0" y="-22.86" drill="1.11" diameter="1.665"/>
<pad name="11" x="0" y="-25.4" drill="1.11" diameter="1.665"/>
<pad name="12" x="0" y="-27.94" drill="1.11" diameter="1.665"/>
<pad name="13" x="0" y="-30.48" drill="1.11" diameter="1.665"/>
<pad name="14" x="0" y="-33.02" drill="1.11" diameter="1.665"/>
<pad name="15" x="0" y="-35.56" drill="1.11" diameter="1.665"/>
<pad name="16" x="0" y="-38.1" drill="1.11" diameter="1.665"/>
<pad name="17" x="-22.86" y="0" drill="1.11" diameter="1.665" shape="square"/>
<pad name="18" x="-22.86" y="-2.54" drill="1.11" diameter="1.665"/>
<pad name="19" x="-22.86" y="-5.08" drill="1.11" diameter="1.665"/>
<pad name="20" x="-22.86" y="-7.62" drill="1.11" diameter="1.665"/>
<pad name="21" x="-22.86" y="-10.16" drill="1.11" diameter="1.665"/>
<pad name="22" x="-22.86" y="-12.7" drill="1.11" diameter="1.665"/>
<pad name="23" x="-22.86" y="-15.24" drill="1.11" diameter="1.665"/>
<pad name="24" x="-22.86" y="-17.78" drill="1.11" diameter="1.665"/>
<pad name="25" x="-22.86" y="-20.32" drill="1.11" diameter="1.665"/>
<pad name="26" x="-22.86" y="-22.86" drill="1.11" diameter="1.665"/>
<pad name="27" x="-22.86" y="-25.4" drill="1.11" diameter="1.665"/>
<pad name="28" x="-22.86" y="-27.94" drill="1.11" diameter="1.665"/>
<pad name="29" x="-22.86" y="-30.48" drill="1.11" diameter="1.665"/>
<pad name="30" x="-22.86" y="-33.02" drill="1.11" diameter="1.665"/>
<pad name="31" x="-22.86" y="-35.56" drill="1.11" diameter="1.665"/>
<pad name="32" x="-22.86" y="-38.1" drill="1.11" diameter="1.665"/>
<pad name="MH1" x="-0.762" y="4.572" drill="2.6" diameter="3.9"/>
<pad name="MH2" x="-0.762" y="-45.974" drill="2.6" diameter="3.9"/>
<pad name="MH3" x="-22.098" y="4.572" drill="2.6" diameter="3.9"/>
<pad name="MH4" x="-22.098" y="-45.974" drill="2.6" diameter="3.9"/>
<text x="-11.43" y="-20.701" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-11.43" y="-20.701" size="1.27" layer="27" align="center">&gt;VALUE</text>
<wire x1="2.413" y1="8.128" x2="-25.273" y2="8.128" width="0.2" layer="51"/>
<wire x1="-25.273" y1="8.128" x2="-25.273" y2="-49.53" width="0.2" layer="51"/>
<wire x1="-25.273" y1="-49.53" x2="2.413" y2="-49.53" width="0.2" layer="51"/>
<wire x1="2.413" y1="-49.53" x2="2.413" y2="8.128" width="0.2" layer="51"/>
<wire x1="2.413" y1="8.128" x2="-25.273" y2="8.128" width="0.1" layer="21"/>
<wire x1="-25.273" y1="8.128" x2="-25.273" y2="-49.53" width="0.1" layer="21"/>
<wire x1="-25.273" y1="-49.53" x2="2.413" y2="-49.53" width="0.1" layer="21"/>
<wire x1="2.413" y1="-49.53" x2="2.413" y2="8.128" width="0.1" layer="21"/>
<wire x1="3.413" y1="9.128" x2="-26.273" y2="9.128" width="0.1" layer="51"/>
<wire x1="-26.273" y1="9.128" x2="-26.273" y2="-50.53" width="0.1" layer="51"/>
<wire x1="-26.273" y1="-50.53" x2="3.413" y2="-50.53" width="0.1" layer="51"/>
<wire x1="3.413" y1="-50.53" x2="3.413" y2="9.128" width="0.1" layer="51"/>
</package>
</packages>
<symbols>
<symbol name="1381">
<wire x1="5.08" y1="10.16" x2="33.02" y2="10.16" width="0.254" layer="94"/>
<wire x1="33.02" y1="-48.26" x2="33.02" y2="10.16" width="0.254" layer="94"/>
<wire x1="33.02" y1="-48.26" x2="5.08" y2="-48.26" width="0.254" layer="94"/>
<wire x1="5.08" y1="10.16" x2="5.08" y2="-48.26" width="0.254" layer="94"/>
<text x="34.29" y="15.24" size="1.778" layer="95" align="center-left">&gt;NAME</text>
<text x="34.29" y="12.7" size="1.778" layer="96" align="center-left">&gt;VALUE</text>
<pin name="ROUT" x="38.1" y="0" length="middle" rot="R180"/>
<pin name="LOUT" x="38.1" y="-2.54" length="middle" rot="R180"/>
<pin name="AGND_1" x="38.1" y="-5.08" length="middle" rot="R180"/>
<pin name="AGND_2" x="38.1" y="-7.62" length="middle" rot="R180"/>
<pin name="GND_1" x="38.1" y="-10.16" length="middle" rot="R180"/>
<pin name="DREQ" x="38.1" y="-12.7" length="middle" rot="R180"/>
<pin name="VCC" x="38.1" y="-15.24" length="middle" rot="R180"/>
<pin name="3V3_1" x="38.1" y="-17.78" length="middle" rot="R180"/>
<pin name="GND_2" x="38.1" y="-20.32" length="middle" rot="R180"/>
<pin name="MISO" x="38.1" y="-22.86" length="middle" rot="R180"/>
<pin name="MOSI" x="38.1" y="-25.4" length="middle" rot="R180"/>
<pin name="SCLK" x="38.1" y="-27.94" length="middle" rot="R180"/>
<pin name="RST" x="38.1" y="-30.48" length="middle" rot="R180"/>
<pin name="CS" x="38.1" y="-33.02" length="middle" rot="R180"/>
<pin name="SDCS" x="38.1" y="-35.56" length="middle" rot="R180"/>
<pin name="XDCS" x="38.1" y="-38.1" length="middle" rot="R180"/>
<pin name="LINE2" x="0" y="0" length="middle"/>
<pin name="MIC+" x="0" y="-2.54" length="middle"/>
<pin name="MIC-" x="0" y="-5.08" length="middle"/>
<pin name="AGND_3" x="0" y="-7.62" length="middle"/>
<pin name="3V3_2" x="0" y="-10.16" length="middle"/>
<pin name="GPIO0" x="0" y="-12.7" length="middle"/>
<pin name="GPIO1" x="0" y="-15.24" length="middle"/>
<pin name="GPIO2" x="0" y="-17.78" length="middle"/>
<pin name="GPIO3" x="0" y="-20.32" length="middle"/>
<pin name="GPIO4" x="0" y="-22.86" length="middle"/>
<pin name="GPIO5" x="0" y="-25.4" length="middle"/>
<pin name="GPIO6" x="0" y="-27.94" length="middle"/>
<pin name="GPIO7" x="0" y="-30.48" length="middle"/>
<pin name="TX" x="0" y="-33.02" length="middle"/>
<pin name="RX" x="0" y="-35.56" length="middle"/>
<pin name="SD_DETECT" x="0" y="-38.1" length="middle"/>
<pin name="MH1" x="20.32" y="15.24" length="middle" rot="R270"/>
<pin name="MH2" x="20.32" y="-53.34" length="middle" rot="R90"/>
<pin name="MH3" x="17.78" y="15.24" length="middle" rot="R270"/>
<pin name="MH4" x="17.78" y="-53.34" length="middle" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="1381" prefix="IC">
<description>&lt;b&gt;Audio IC Development Tools VS1053 Codec + MicroSD Breakout - MP3/WAV/MIDI/OGG Play + Record - v4&lt;/b&gt;&lt;p&gt;
Source: &lt;a href="https://cdn-shop.adafruit.com/datasheets/vs1053.pdf"&gt; Datasheet &lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="1381" x="0" y="0"/>
</gates>
<devices>
<device name="" package="1381">
<connects>
<connect gate="G$1" pin="3V3_1" pad="8"/>
<connect gate="G$1" pin="3V3_2" pad="21"/>
<connect gate="G$1" pin="AGND_1" pad="3"/>
<connect gate="G$1" pin="AGND_2" pad="4"/>
<connect gate="G$1" pin="AGND_3" pad="20"/>
<connect gate="G$1" pin="CS" pad="14"/>
<connect gate="G$1" pin="DREQ" pad="6"/>
<connect gate="G$1" pin="GND_1" pad="5"/>
<connect gate="G$1" pin="GND_2" pad="9"/>
<connect gate="G$1" pin="GPIO0" pad="22"/>
<connect gate="G$1" pin="GPIO1" pad="23"/>
<connect gate="G$1" pin="GPIO2" pad="24"/>
<connect gate="G$1" pin="GPIO3" pad="25"/>
<connect gate="G$1" pin="GPIO4" pad="26"/>
<connect gate="G$1" pin="GPIO5" pad="27"/>
<connect gate="G$1" pin="GPIO6" pad="28"/>
<connect gate="G$1" pin="GPIO7" pad="29"/>
<connect gate="G$1" pin="LINE2" pad="17"/>
<connect gate="G$1" pin="LOUT" pad="2"/>
<connect gate="G$1" pin="MH1" pad="MH1"/>
<connect gate="G$1" pin="MH2" pad="MH2"/>
<connect gate="G$1" pin="MH3" pad="MH3"/>
<connect gate="G$1" pin="MH4" pad="MH4"/>
<connect gate="G$1" pin="MIC+" pad="18"/>
<connect gate="G$1" pin="MIC-" pad="19"/>
<connect gate="G$1" pin="MISO" pad="10"/>
<connect gate="G$1" pin="MOSI" pad="11"/>
<connect gate="G$1" pin="ROUT" pad="1"/>
<connect gate="G$1" pin="RST" pad="13"/>
<connect gate="G$1" pin="RX" pad="31"/>
<connect gate="G$1" pin="SCLK" pad="12"/>
<connect gate="G$1" pin="SDCS" pad="15"/>
<connect gate="G$1" pin="SD_DETECT" pad="32"/>
<connect gate="G$1" pin="TX" pad="30"/>
<connect gate="G$1" pin="VCC" pad="7"/>
<connect gate="G$1" pin="XDCS" pad="16"/>
</connects>
<technologies>
<technology name="">
<attribute name="ARROW_PART_NUMBER" value="1381" constant="no"/>
<attribute name="ARROW_PRICE-STOCK" value="https://www.arrow.com/en/products/1381/adafruit-industries?region=nac" constant="no"/>
<attribute name="DESCRIPTION" value="Audio IC Development Tools VS1053 Codec + MicroSD Breakout - MP3/WAV/MIDI/OGG Play + Record - v4" constant="no"/>
<attribute name="HEIGHT" value="5mm" constant="no"/>
<attribute name="MANUFACTURER_NAME" value="Adafruit" constant="no"/>
<attribute name="MANUFACTURER_PART_NUMBER" value="1381" constant="no"/>
<attribute name="MOUSER_PART_NUMBER" value="485-1381" constant="no"/>
<attribute name="MOUSER_PRICE-STOCK" value="https://www.mouser.co.uk/ProductDetail/Adafruit/1381?qs=GURawfaeGuC8%2Fdp%252B1TGxFQ%3D%3D" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="IC1" library="SamacSys_Parts" deviceset="1381" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="IC1" gate="G$1" x="27.94" y="71.12" smashed="yes">
<attribute name="NAME" x="62.23" y="86.36" size="1.778" layer="95" align="center-left"/>
<attribute name="VALUE" x="62.23" y="83.82" size="1.778" layer="96" align="center-left"/>
</instance>
</instances>
<busses>
</busses>
<nets>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
