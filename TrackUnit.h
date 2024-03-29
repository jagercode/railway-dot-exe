//TrackUnit.h
//Comments in .h files are believed to be accurate and up to date
/*
This is a source code file for "railway.exe", a railway operation
simulator, written in Borland C++ Builder 4 Professional
Copyright (C) 2010 Albert Ball [original development]

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------
#ifndef TrackUnitH
#define TrackUnitH
//---------------------------------------------------------------------------

#include <vcl.h>
#include <vector>
#include <map>
#include <fstream>
#include <list>
#include <utility> //for pair

#define FirstUnusedSpeedTagNumber 145 //defined value for use in array sizing etc
#define DefaultTrackLength 100
#define DefaultTrackSpeedLimit 200


//---------------------------------------------------------------------------

typedef std::pair<int, int> THVPair;//HLoc/VLoc position pair

class TDisplay;//type only declared here to allow access, full declaration in DisplayUnit

//---------------------------------------------------------------------------

class TMapComp //map and multimap comparator based on horizontal & vertical position
{
public:
bool operator() (const THVPair& lower, const THVPair& higher) const;//HLoc  VLoc
};

//---------------------------------------------------------------------------

enum TTrackType {Simple, Crossover, Points, Buffers, Bridge, SignalPost, Continuation, Platform, //describes the type of track element
        GapJump, Footbridge, Unused,//'unused' was marker the for old 'text' number, since disused
        Concourse, Parapet, NamedNonStationLocation,//these 3 as well as Platform are the 4 types of inactive element
        Erase,//default active element used for erasing, all data members unset
        LevelCrossing};
enum TConfiguration {NotSet, Connection, End, Gap, Lead, Trail, CrossConn, Under, Signal}; //describes the type of track link
                                                                                           //'End' is used for both buffer stop and
                                                                                           //continuation entry/exit positions
//FIXED TRACK :-

class TFixedTrackPiece//All basic track building blocks & methods
    {
    public://everything uses these - should really have Gets & Sets but too many to change now

    bool FixedNamedLocationElement;     //true for an element that can be named (platforms, concourse, footbridges &
                                            //non-station named loactions)
    int SpeedTag;                       //the element identification number - corresponds to the relevant SpeedButton->Tag
    int Link[4];                        //track connection link values, max. of 4, unused = -1, top lh diag.=1, top=2, top rh diag.=3
                                            //left=4, right=6, bottom lh diag.=7, bottom=8, bottom rh diag.=9
    Graphics::TBitmap *GraphicPtr;      //the track bitmap for display on the zoomed-in railway
    Graphics::TBitmap *SmallGraphicPtr; //the track bitmap for display on the zoomed-out railway

    TConfiguration Config[4];           //the type of link - see TConfiguration above

    TTrackType TrackType;               //the type of track element


    void PlotFixedTrackElement(int Caller, int HLocInput, int VLocInput) const; //plot the element on the railway display at position
                                                                                    //HLocInput & VLocInput
    TFixedTrackPiece(int SpeedTagVal, TTrackType TrackTypeVal, int LkVal[4], //constructor for building TTrack.FixedTrackArray - see below
            TConfiguration ConfigVal[4], Graphics::TBitmap *GraphicPtrVal, Graphics::TBitmap *SmallGraphicPtrVal);
    TFixedTrackPiece(); //default constructor
    };

//---------------------------------------------------------------------------

//VARIABLE TRACK :-

//---------------------------------------------------------------------------

/*Note:  Should probably have used different derived classes for the different track types, to avoid all having
attributes & other specific data, but by the time this occurred to me as a good idea it seemed likely to be more difficult
to backtrack than to put up with the extra & unused data.
*/

class TTrackElement : public TFixedTrackPiece  //Basic track elements as implemented in the overall railway layout
    {
    public://everything uses these - should really have Gets & Sets but too many to change now

    AnsiString ActiveTrackElementName; //location name used either in the timetable or for a continuation (continuation names not used in
                                            //timetable as trains can't stop there).  Only active track elements where there are platforms
                                            // or non-station named locations have ActiveTrackElementNames
    AnsiString ElementID; //the element identifier based on position in the railway
    AnsiString LocationName; //location name not used for timetabling, only for identification: platforms, non-station named locations,
                                //concourses and footbridges have LocationNames

    bool CallingOnSet; //used for for signals only when a train is being called on - used to plot the position lights
    bool TempMarker; //utility marker for program use
    bool TempTrackMarker01, TempTrackMarker23; //utility markers for program use

    int Attribute; //special variable used only for points & signals, ignored otherwise
        //points:  0=set to go straight, 1=set to diverge; where both legs diverge 0=set to left fork
        //signals:  0=red; 1=yellow; 2=double yellow; 3 = green;
        //Level crossing: 0 = raised barriers = closed to trains; 1 = lowered barriers = open to trains; 2 = changing state = closed to trains
    int Conn[4]; //connecting element position in TrackVector, set to -1 if no connecting link or if track not linked
    int ConnLinkPos[4]; //connecting element link position (i.e. array positions of the connecting element links, in same order as Link[4])
    int HLoc, VLoc; //The h & v locations in the railway (top lh corner of the first build screen = 0,0)
    int Length01, Length23, SpeedLimit01, SpeedLimit23; //element lengths and speed limits, ...01 is for the track with link positions [0]
                                                            //and [1], ...23 for [2] and [3], set to -1 if not used (lengths in m & speed
                                                            //limits in km/h)
    int StationEntryStopLinkPos1, StationEntryStopLinkPos2; //used for track at platforms and non-station named locations to mark the
                                                                //train front element stop position, there are two for the two directions
                                                                //of travel, set to -1 if not used
    int TrainIDOnElement, TrainIDOnBridgeTrackPos01, TrainIDOnBridgeTrackPos23; //set to the TrainID value when a train is present on the
                                                                                    //element, bridges can have two trains present so the
                                                                                    //...01 and ...23 values give the TrainIDs for track
                                                                                    //with link positions [0] & [1], and [2] & [3]
                                                                                    //respectively
    enum {FourAspect, ThreeAspect, TwoAspect, GroundSignal} SigAspect; //added at version 0.6
//inline functions
    TTrackElement(TFixedTrackPiece Input) : TFixedTrackPiece(Input), HLoc(-2000000000), VLoc(-2000000000),
            LocationName(""), ActiveTrackElementName(""), Attribute(0), CallingOnSet(false), Length01(DefaultTrackLength), Length23(-1),
            SpeedLimit01(DefaultTrackSpeedLimit), SpeedLimit23(-1), TrainIDOnElement(-1), TrainIDOnBridgeTrackPos01(-1),
            TrainIDOnBridgeTrackPos23(-1), StationEntryStopLinkPos1(-1), StationEntryStopLinkPos2(-1), SigAspect(FourAspect)
            //constructor for specific type of element, use very high neg. numbers as 'unset' values for HLoc & VLoc initially as can go
                //high negatively legitimately, build from existing TTrackPiece with default values for extra members

        {
        for(int x=0;x<4;x++)
            {
            ConnLinkPos[x]=-1; Conn[x]=-1;
            }
        if((TrackType == Points) || (TrackType == Crossover) || (TrackType == Bridge))
            {
            Length23 = DefaultTrackLength;

            SpeedLimit23 = DefaultTrackSpeedLimit;
            }
        }

    TTrackElement() : TFixedTrackPiece(), HLoc(-2000000000), VLoc(-2000000000), LocationName(""), ActiveTrackElementName(""),
            Attribute(0), CallingOnSet(false), Length01(-1), Length23(-1), SpeedLimit01(-1), SpeedLimit23(-1), TrainIDOnElement(-1),
            TrainIDOnBridgeTrackPos01(-1), TrainIDOnBridgeTrackPos23(-1), StationEntryStopLinkPos1(-1), StationEntryStopLinkPos2(-1),
            SigAspect(FourAspect)
            //constructor for non-specific default element - use high neg numbers for 'unset' h & v as can go high negatively legitimately
        {
        for(int x=0;x<4;x++)
            {
            ConnLinkPos[x]=-1;
            Conn[x]=-1;
            }
        }

//functions defined in .cpp file
    bool operator == (TTrackElement RHElement); //equivalence operator
    bool operator != (TTrackElement RHElement); //non-equivalence operator

    AnsiString TTrackElement::LogTrack(int Caller) const;//used to log track parameters for call stack logging

    void PlotVariableTrackElement(int Caller, TDisplay *Disp) const; //plot the element on the display - 'variable' indicates that the
                                                                        //element may be named and if so may be plotted striped or solid
                                                                        //depending on whether the name has been set
};

//---------------------------------------------------------------------------
//PrefDir & Route elements
//---------------------------------------------------------------------------
class TPrefDirElement : public TTrackElement //basic preferred direction or route element - track element with additional members
    {
    protected:

    int ELink, ELinkPos; //entry link number & array position
    int XLink, XLinkPos; //exit link number & array position
    int EXNumber; //used to facilitate identification of the appropriate preferred direction or route graphic
    int TrackVectorPosition; //TrackVectorPosition of the corresponding track element
    int CheckCount; //internal check value used when building preferred directions
    Graphics::TBitmap *EXGraphicPtr, *EntryDirectionGraphicPtr; //pointers to the appropriate entry/exit graphic, or direction marker
                                                                //graphic, for preferred directions and routes

    bool operator == (TPrefDirElement RHElement); //equivalence operator
    bool operator != (TPrefDirElement RHElement); //non-equivalence operator

    bool EntryExitNumber(); //determines and loads EXNumber (see above)
    Graphics::TBitmap *GetDirectionPrefDirGraphicPtr() const; //picks up the EntryDirectionGraphicPtr for preferred directions
    Graphics::TBitmap *GetDirectionRouteGraphicPtr(bool AutoSigsFlag, bool ConsecSignalsRoute) const; //picks up the green or
                                                                                                      //red route direction graphic
    Graphics::TBitmap *GetOriginalGraphicPtr(); //picks up the original (non-flashing) graphic for use during route flashing
    Graphics::TBitmap *GetPrefDirGraphicPtr(); //picks up the EXGraphicPtr for preferred directions
    Graphics::TBitmap *GetRouteAutoSigsGraphicPtr(); //picks up the blue route graphic (not used - superseded by GetRouteGraphicPtr)
    Graphics::TBitmap *GetRouteGraphicPtr(bool AutoSigsFlag, bool ConsecSignalsRoute); //picks up the appropriate route graphic

    public:

    friend class TOnePrefDir;
    friend class TOneRoute;
    friend class TAllRoutes;

    bool IsARoute;  //false for Pref Dir, true for route
    bool AutoSignals;  //marker within the route for an AutoSignal route element
    bool ConsecSignals;//marker within the route for ConsecSignalsRoute element

//inline functions
    bool IsPosition(int Position) const {if(TrackVectorPosition == Position) return true; else return false;} //position check
    int GetELink() const {return ELink;} //returns ELink
    int GetELinkPos() const {return ELinkPos;} //returns the ELink array position
    int GetXLink() const {return XLink;} //returns XLink
    int GetXLinkPos() const {return XLinkPos;} //returns the XLink array position
    unsigned int GetTrackVectorPosition() const {return TrackVectorPosition;} //returns TrackVectorPosition
    Graphics::TBitmap *GetEXGraphicPtr() {return GetPrefDirGraphicPtr();} //returns EXGraphicPtr for preferred directions
    Graphics::TBitmap *GetRouteEXGraphicPtr() {return GetRouteGraphicPtr(AutoSignals, ConsecSignals);} //returns route graphic

    TPrefDirElement() : TTrackElement(), ELink(-1), ELinkPos(-1), //default constructor, loads default values
        XLink(-1), XLinkPos(-1), EXNumber(-1), TrackVectorPosition(-1), CheckCount(0),
        EXGraphicPtr(0), EntryDirectionGraphicPtr(0), IsARoute(false), AutoSignals(false), ConsecSignals(false) {;}
    TPrefDirElement(TTrackElement Input) : TTrackElement(Input), ELink(-1), ELinkPos(-1), //constructs a PrefDirElement from a base
                                                                                          //TrackElement, sets up the TrackElement values
                                                                                          //but leaves others as default values
        XLink(-1), XLinkPos(-1), EXNumber(-1), TrackVectorPosition(-1),
        CheckCount(0), EXGraphicPtr(0), EntryDirectionGraphicPtr(0), IsARoute(false),  AutoSignals(false), ConsecSignals(false) {;}

//external functions
    AnsiString LogPrefDir() const; //sends a list of PrefDirElement values to Utilities->CallLog file for debugging purposes

    TPrefDirElement(TTrackElement InputElement, int ELink, int ELinkPos, int XLink, int XLinkPos, int TrackVectorPosition);//constructs a
                                                                                                                           //PrefDirElement
                                                                                                                           //from supplied
                                                                                                                           //values
    };

//---------------------------------------------------------------------------

class TGraphicElement
/*
Allows a single Width x Height graphic to change and change back independently of the remaining display.  Used for the flashing green and
red gap markers, flashing points and route start graphics.  The code is mostly self-explanatory, but SetScreenHVSource (sets source
rectangle) must be called before the original graphic is loaded, whether or not the graphic is loaded from the screen (using
LoadOriginalScreenGraphic, for point flashing and route start markers) or an existing bitmap (using LoadOriginalExistingGraphic, for red
and green gap flashing), and OverlayGraphic and OriginalGraphic must be loaded before they are plotted.  Checks are
built in for these conditions.  SourceRect is the rectangle on the appropriate canvas where the original graphic is taken from.  The
original graphic can be taken from the screen - LoadOriginalScreenGraphic(), or from a section from an existing bitmap -
LoadOriginalExistingGraphic.  If an existing bitmap is selected then the loading function overrides the size that was set in the
constructor, and SourceRect & HPos & VPos that were set in SetScreenHVSource.
*/
    {
    private:

    bool OverlayPlotted, OverlayLoaded, OriginalLoaded, ScreenSourceSet, ScreenGraphicLoaded, ExistingGraphicLoaded; //state flags
    int HPos, VPos; //horizontal and vertical positions
    int Width, Height; //dimensions in pixels
    Graphics::TBitmap *OriginalGraphic, *OverlayGraphic; //original and temporary overlay graphics
    TRect SourceRect; //source rectangle of the original graphic

    public:
    //inline functions
    int GetHPos() {return HPos;} //return horizontal position
    int GetVPos() {return VPos;} //return vertical position
    void SetSourceRect(int Left, int Top) //set SourceRect member values from those supplied and existing Width & Height - ensure this is
                                          //only called after Width & Height are set
        {
        SourceRect.Left = Left;
        SourceRect.Right = Left + Width;
        SourceRect.Top = Top;
        SourceRect.Bottom = Top + Height;
        }

    //functions defined in .cpp file
    void LoadOriginalExistingGraphic(int Caller, int HOffset, int VOffset, int WidthIn, int HeightIn, //load red or green gap flashing
                                                                                                      //graphic from the stored bitmaps
            Graphics::TBitmap *Graphic);
    void LoadOriginalScreenGraphic(int Caller); //load original graphic from the screen for point flashing or route start markers
    void LoadOverlayGraphic(int Caller, Graphics::TBitmap *Overlay); //load the temporary overlay graphic
    void PlotOverlay(int Caller, TDisplay *Disp); //plot the overlay graphic on screen
    void PlotOriginal(int Caller, TDisplay *Disp); //plot the original graphic on screen
    void SetScreenHVSource(int Caller, int HPosIn, int VPosIn); //set HPos, VPos & SourceRect member values from the supplied positions
    TGraphicElement();//default constructor (16 x 16 pixel element)
    TGraphicElement(int WidthIn, int HeightIn); //constructor for specified dimensions
    ~TGraphicElement(); //destructor
    };

//---------------------------------------------------------------------------

class IDInt //this was introduced when it was decided to have a route identification number for each route rather than using the vector
    //position number for identifying existing routes that were being extended during route building.  Using vector position numbers
    //meant that these identification numbers had to be changed when existing routes were erased by trains passing over them.  IDInt is
    //used for StartSelectionRouteID and ReqPosRouteID (see TAllRoutes) and ensures that any confusion with the old vector position
    //numbers is picked up by the compiler. Note that the route's RouteID value is an 'int', not an 'IDInt', 'IDInt' is only used for
    //StartSelectionRouteID and ReqPosRouteID
{
private:
int InternalInt; //the internal integer value represented by IDInt

public:
//all inline
int GetInt() const {return InternalInt;} //return the internal integer
bool operator ==(IDInt Comparator) {return (InternalInt == Comparator.InternalInt);} //equality comparator
bool operator >(int Comparator) {return (InternalInt > Comparator);} //greater than comparator
explicit IDInt::IDInt(int Int) {InternalInt = Int;} //constructor that sets the internal integer to the input value. The 'explicit' prefix
    //is used to force a compiler error if the input value is an IDInt, which would be a program error (otherwise it would be implicitly
    //converted to an int)
IDInt::IDInt() {InternalInt = -1;} //default constructor, internal integer set to -1
};

//---------------------------------------------------------------------------
//Track
//---------------------------------------------------------------------------

class TTrack  //All dynamic track data & methods.  Only one object since only one operating railway

/*Note:  The TrackMap & InactiveTrackMap were developed well after the TrackVector, to speed up track element
searches.  It was realised at that time that the maps themselves could contain type TTrackElement rather than int
(for TrackVectorPosition), and that the track vectors could be dispensed with completely.  However after an attempt
to remove them it was clear that they were far too embedded throughout the program for easy removal, so they were
left in.
*/
    {
    private:

    class TFixedTrackArray //a class that holds an array of TrackPieces, only accessible to TTrack
        {
        public:

        TFixedTrackPiece FixedTrackPiece[FirstUnusedSpeedTagNumber]; //the array member
        TFixedTrackArray(); //array constructor
        };

    TFixedTrackArray FixedTrackArray; //the FixedTrackPiece array object

    TTrackElement DistanceStartElement, DistanceContinuingElement; //initially used for track element lengths but since disused

    bool TrackFinished; //marker for all Conn & ConnLinkPos values set & track complete

    int GapPos, GapHLoc, GapVLoc; //record gap setting info
    int HLocMin, VLocMin, HLocMax, VLocMax; //give extent of railway for use in zoomed in and out displays and in saving railway images
    int LinkCheckArray[9][2]; //array of valid link connecting values, I don't think this is used now
    int LinkHVArray[10][2]; //array used to determine relative horizontal & vertical track element positions for specific link values
    int Tag76Array[23][3]; //these arrays give valid adjacent named element relative positions for each type of named element,
    int Tag77Array[23][3]; //the numbers - 76, 77 etc - relate to track element element SpeedTag values (76 - 79 = platforms, 96 =
    int Tag78Array[23][3]; //concourse, 129 & 130 = footbridges and 131 = non-station named location.
    int Tag79Array[23][3];
    int Tag96Array[24][3];
    int Tag129Array[8][3];
    int Tag130Array[8][3];
    int Tag131Array[4][3];

    Set<int, 1, 130> TopPlatAllowed, BotPlatAllowed, LeftPlatAllowed, RightPlatAllowed, NameAllowed, LevelCrossingAllowed; //sets of valid TrackElements for
                                                                                                     //placement of platforms and
                                                                                                     //non-station named locations
    public:

    enum TBarrierState {Raising, Lowering, Up, Down}; //state of barriers

    class TActiveLevelCrossing //values for level crossings either changing state or with barriers down
    //All LCs begin with barriers raised. i.e. closed to trains, that is the normal state.  When a route is set through an LC nothing happens until the
    //route is set, then an active LC object is created by SetLCChangeValues (called by ConvertandAdd....  for lowering barriers) and added to the
    //ChangingLCVector.  Once created 'FlashingGraphics' takes care of the flashing, until the duration is reached.  While flashing no further routes
    //can be set through that LC and the first route can't be cancelled, hence the flashing only needs to cater for plotting the route on the one track that
    //started the barrier lowering.  When the duration is reached, the object is transferred to a new vector BarriersDownVector, after the StartTime has been
    //reset (to time the period for which the barriers are down - penalties are given for > 3 minutes), BarrierState changed to Down, and the object erased
    //from ChangingLCVector.  When there is no route through an LC and no train on the track then the barriers are raised - in ClockTimer2 - when the
    //BarriersDownVector object is copied back to ChangingLCVector with a new StartTime, BarrierState and ChangeDuration.  Again FlashingGraphics takes care
    //of the flashing until the duration is reached, when the object is erased from the vector and the LC reverts to its normal (barriers raised) state.
        {
        public:

        bool ConsecSignals;         //route type - 0 = nonsignals, 1 = preferred direction (can't have autosigs)
        bool TrainPassed;           //marker that is set when a train is present on one of the elements of the LC - used to provide a 3 minute penalty allowance
        TBarrierState BarrierState; //state of barriers
        float ChangeDuration;       //duration of the level crossing changing period
        int BaseElementSpeedTag;    //SpeedTag value for the base element of a level crossing
        int HLoc;                   //HLoc value for found level crossing element
        int VLoc;                   //VLoc value for found level crossing element
        TDateTime StartTime;        //stores the starting time for level crossing changing
        TActiveLevelCrossing::TActiveLevelCrossing(); //constructor, sets default values
        };

    typedef std::vector<TActiveLevelCrossing> TActiveLCVector; //vector of changing level crossing objects.  Note that although a LC may contain several
    //elements there will be only one in the vector when changing, and it might be any of the individual elements.  This is because when an entry
    //is made all linked elements have their attributes changed to 2 for changing, so no more are found.  This applies both for closing & opening to trains

    typedef std::vector<int> TLCVector; //vector of level crossing InactiveTrackVector positions - note that this contains all LC elements whether
                                        //linked to others or not

    typedef std::vector<TTrackElement> TTrackVector; //vector of TrackElements
    typedef std::vector<TTrackElement>::iterator TTrackVectorIterator;

    typedef std::map<THVPair, unsigned int, TMapComp> TTrackMap; //map of TrackElement TrackVectorPositions, HLoc & VLoc pair is the key
    typedef TTrackMap::iterator TTrackMapIterator;
    typedef std::pair<THVPair, unsigned int> TTrackMapEntry;

    typedef std::map<THVPair, THVPair, TMapComp> TGapMap; //map of matching gap positions as an HLoc/VLoc pair, with the key being
    typedef TGapMap::iterator TGapMapIterator;            //the first gap HLoc/VLoc pair, contains one entry for each pair of matched gaps
    typedef std::pair<THVPair, THVPair> TGapMapEntry;

    typedef std::multimap<THVPair, unsigned int, TMapComp> TInactiveTrack2MultiMap; //multimap of inactive TrackElements (platforms,
    typedef TInactiveTrack2MultiMap::iterator TInactiveTrack2MultiMapIterator;      //concourses, non-station named locations & parapets)
    typedef std::pair<TInactiveTrack2MultiMapIterator, TInactiveTrack2MultiMapIterator> TInactiveTrackRange;//'2' because there can be up
                                                                                                            //to 2 entries (platforms) at
                                                                                                            //a single location

    typedef std::pair<unsigned int, unsigned int> TIMPair; //TrackElement pair type used for inactive elements, values are vector positions

    typedef std::list<int> TLNPendingList;                   //type list of location name vector positions (see note below) used during
    typedef TLNPendingList::iterator TLNPendingListIterator; //naming of linked named location elements

    typedef std::multimap<THVPair, int, TMapComp> TLNDone2MultiMap; //multimap of location name vector positions (see note below) used
    typedef TLNDone2MultiMap::iterator TLNDone2MultiMapIterator;    //during naming of linked named location elements, '2' because there
    typedef std::pair<THVPair, int> TLNDone2MultiMapEntry;          //can be up to 2 entries (platforms) at a single location

    typedef std::multimap<AnsiString, int> TLocationNameMultiMap;//map of location name vector positions (see note below), one entry for
    //every element that is a FixedNamedLocationElement i.e platforms, concourses, footbridges & named non-station locations.  Hence the
    //only active track elements included are footbridges
    typedef TLocationNameMultiMap::iterator TLocationNameMultiMapIterator;
    typedef std::pair<TLocationNameMultiMapIterator, TLocationNameMultiMapIterator> TLocationNameMultiMapRange;
    typedef std::pair<AnsiString, int> TLocationNameMultiMapEntry;

    //NOTE: the above (TLNPendingList, TLNDone2MultiMap & TLocationNameMultiMap) store adjusted vector positions - adjusted because have
    //a single int to represent both active and inactive vector positions.  Use (-1 - Position) for active vector positions & (Position)
    //for inactive vector positions (most location elements are in the inactive vector so these are positive).  Tthe '-1' bit is needed
    //because the value '0' is used for the first position in the inactive vector

    typedef std::map<AnsiString, int> TActiveTrackElementNameMap; //map of ActiveTrackElementNames compiled and used for populating the
        //LocationNameComboBox during timetable creation or editing.  Used in place of LocationNameMultiMap as that can contain
        //concourses and non-station named locations that aren't associated with any track.  The second 'int' entry is a dummy, only
        //the list of AnsiString names is needed, and being a map it is automatically sorted and without duplicates.
    typedef TActiveTrackElementNameMap::iterator TActiveTrackElementNameIterator;
    typedef std::pair<AnsiString, int> TActiveTrackElementNameMapEntry;

    struct TSigElement //used as basic elements in a table of signals - see SigTable below
        {//NOTE: Don't alter the order of these members as they are loaded from an array of values in the constructor
        int SpeedTag; //the TrackElement SpeedTag value - specifies the signal element
        int Attribute; //the signal state - red, yellow, double yellow or green
        Graphics::TBitmap* SigPtr; //pointer to the graphic
        };

    TSigElement SigTable[40]; //original table of signals for four aspect
    TSigElement SigTableThreeAspect[40]; //new at version 0.6 for three aspect
    TSigElement SigTableTwoAspect[40]; //new at version 0.6 for two aspect
    TSigElement SigTableGroundSignal[40]; //new at version 0.6 for ground signals

    AnsiString RouteFailMessage;

    bool ActiveTrackElementNameMapCompiledFlag; //indicates that the ActiveTrackElementNameMap has been compiled
    bool GapFlashFlag; //true when a pair of connected gaps is flashing
    bool LCChangeFlag; //true when LCs changing
    bool LCFoundInAutoSigsRoute; //true if found an LC during an automatic route search
    bool LCFoundInAutoSigsRouteMessageGiven; //true if message given to user, to avoid giving multiple times and to avoid other failure messages being given
    bool LCFoundInRouteBuildingFlag; //true if a route set through an LC that is closed to trains (& therefore needs to be opened)
    bool PointFlashFlag; //true when points are flashing during manual change
    bool RouteFlashFlag; //true while a route is flashing prior to being set

    float LevelCrossingBarrierUpFlashDuration;//duration of the flash period when level crossing closing to trains
    float LevelCrossingBarrierDownFlashDuration;//duration of the flash period when level crossing opening

    int FlipArray[FirstUnusedSpeedTagNumber]; //holds TrackElement SpeedTag values for 'flipping' via menu items 'Edit' & 'Flip'
    int GapFlashGreenPosition, GapFlashRedPosition;//TrackVectorPosition of the gap element that is flashing green or red
    int MirrorArray[FirstUnusedSpeedTagNumber]; //holds TrackElement SpeedTag values for 'mirroring' via menu items 'Edit' & 'Mirror'
    std::map<AnsiString, char> ContinuationNameMap; //map of all continuation names, char is a dummy
    TActiveTrackElementNameMap ActiveTrackElementNameMap; //map of active track element names
    TActiveLCVector ChangingLCVector; //vector of values for changing level crossings - i.e. barriers in course of being raised or lowered
    TActiveLCVector BarriersDownVector; //vector of LCs with barriers down
    TGapMap GapMap; //map of gaps (see type for more information above)
    TGraphicElement *GapFlashGreen, *GapFlashRed; //the red & green circle graphics used to show where the gaps are
    TInactiveTrack2MultiMap InactiveTrack2MultiMap; //multimap of inactive TrackElements (see type for more information above)
    TLCVector LCVector; //vector of level crossing InactiveTrackVector positions
    TLNDone2MultiMap LNDone2MultiMap; //multimap of processed location name elements (see type for more information above)
    TLNPendingList LNPendingList; //list of location name elements awaiting processing (see type for more information above)
    TLocationNameMultiMap LocationNameMultiMap; //multimap of location names (see type for more information above)
    TTrackMap TrackMap; //map of track (see type for more information above)
    TTrackVector TrackVector, InactiveTrackVector, NewVector, DistanceVector, DistanceSearchVector, SelectVector;//vectors of TrackElements
    TTrackVectorIterator NextTrackElementPtr; //track vector iterator used during cycling through a track vector

//inline functions
    AnsiString GetLocationName(unsigned int InactiveTrackVectorPosition) {return InactiveTrackElementAt(24, InactiveTrackVectorPosition).LocationName;}
        //return location name for a given inactive track vector position
    bool IsReadyForOperation() {return (IsTrackFinished() && !LocationsNotNamed(1) && !GapsUnset(8));} //indicates whether or not the
                                                                                                       //is ready for saving as a '.rly'
                                                                                                       //file and for operation
    bool IsTrackFinished() {return TrackFinished;} //indicates whether or not the track has been successfully linked together

    enum {FourAspectBuild, ThreeAspectBuild, TwoAspectBuild, GroundSignalBuild} SignalAspectBuildMode; //aspect mode for future signal additions

    int GetGapHLoc() {return GapHLoc;} //return the respective values
    int GetGapVLoc() {return GapVLoc;}
    int GetHLocMax() {return HLocMax;}
    int GetHLocMin() {return HLocMin;}
    int GetVLocMax() {return VLocMax;}
    int GetVLocMin() {return VLocMin;}
    int GetNonPointsOppositeLinkPos(int LinkPosIn) //return the corresponding link position (track always occupies either links 0 & 1 or 2 & 3)
        {
        if(LinkPosIn == 3) return 2;
        if(LinkPosIn == 2) return 3;
        if(LinkPosIn == 1) return 0;
        return 1;
        }
    int TrackVectorSize() {return TrackVector.size();} //return the number of active track elements
    unsigned int SelectVectorSize() {return SelectVector.size();} //return the number of selected active and inactive track elements (via
                                                                  //menu items 'Edit' and 'Select')
    void SelectPush(TTrackElement TrackElement) {SelectVector.push_back(TrackElement);} //store a TrackElement in the SelectVector
    void SelectVectorClear() {SelectVector.clear();} //clear the SelectVector
    void SetHLocMax(int HLoc) {HLocMax = HLoc;} //set member values
    void SetHLocMin(int HLoc) {HLocMin = HLoc;}
    void SetTrackFinished(bool Value) {TrackFinished = Value;}
    void SetVLocMax(int VLoc) {VLocMax = VLoc;}
    void SetVLocMin(int VLoc) {VLocMin = VLoc;}

//externally defined functions
    bool ActiveMapCheck(int Caller, int HLoc, int VLoc, int SpeedTag); //used to check the validity of footbridge links
    bool AnyLinkedLevelCrossingElementsWithRoutesOrTrains(int Caller, int HLoc, int VLoc, bool &TrainPresent); //true if a route or train present on
        //any linked level crossing element
    bool AdjElement(int Caller, int HLoc, int VLoc, int SpeedTag, int &FoundElement); //used during location naming to check for adjacent
        //named elements to a given element at HLoc & VLoc with a specific SpeedTag, and if found allow that element to be inserted
        //into the LNPendingList for naming similarly
    bool AdjNamedElement(int Caller, int HLoc, int VLoc, int SpeedTag, AnsiString &LocationName, int &FoundElement); //used in
        //SearchForAndUpdateLocationName to check for adjacent named elements to a given element at HLoc & VLoc with a specific SpeedTag,
        //and if found allow that name to be used for this element and all other named elements that are linked to it
    bool BlankElementAt(int Caller, int At) const; //true for a blank (SpeedTag == 0) element at a specific Trackvector position,
        //no longer used after TrackErase changed (now EraseTrackElement) so that blank elements aren't used
    bool CheckActiveLCVector(int Caller, std::ifstream &VecFile); //true if BarriersDownVector checks OK in SessionFile
    bool CheckFootbridgeLinks(int Caller, TTrackElement &TrackElement); //true if a footbridge is linked properly at both ends
    bool CheckOldTrackElementsInFile(int Caller, int &NumberOfActiveElements, std::ifstream& VecFile); //version of CheckTrackElementsInFile
        //prior to its being updated, used when changes made to CheckTrackElementsInFile in order to allow existing saved railways to be loaded prior
        //to resaving in the new format compatible with the new CheckTrackElementsInFile
    bool CheckTrackElementsInFile(int Caller,  int &NumberOfActiveElements, std::ifstream& VecFile); //true if TrackElements in the file
        //are all valid
    bool DiagonalFouledByTrain(int Caller, int HLoc, int VLoc, int DiagonalLinkNumber, int &TrainID); //as DiagonalFouledByRouteOrTrain (in TAllRoutes) but
        //only checks for a train (may or may not be a route present (new at v1.2.0)
    bool ElementInLNDone2MultiMap(int Caller, int MapPos); //true if the element defined by MapPos is present in LNDone2MultiMap, used during
        //location naming
    bool ElementInLNPendingList(int Caller, int MapPos); //true if the element defined by MapPos is present in LNPendingList, used during
        //location naming
    bool ErrorInTrackBeforeSetGaps(int Caller, int &HLoc, int &VLoc); //check for track errors prior to gap setting - disused as
        //incorporated a time-consuming double brute force search
    bool FindAndHighlightAnUnsetGap(int Caller); //true if there is an unset gap, and if so it is marked with a red circle, used during
        //gap setting
    bool FindHighestLowestAndLeftmostNamedElements(int Caller, AnsiString Name, int &VPosHi, int &VPosLo, int &HPos); //used in locating
        //the screen name position for a named location, return true if find an inactive element called 'Name'
    bool FindNonPlatformMatch(int Caller, int HLoc, int VLoc, int &Position, TTrackElement &TrackElement); //true if find a non-platform
        //element at HLoc & VLoc, and if so return its TrackVector position and a reference to it in TrackElement
    bool FindSetAndDisplayMatchingGap(int Caller, int HLoc, int VLoc); //true if find an unset gap that matches the gap at HLoc & VLoc,
        //if find one mark it with a green circle
    bool GapsUnset(int Caller); //true if there are gaps in the railway and any are unset
    bool InactiveMapCheck(int Caller, int HLoc, int VLoc, int SpeedTag); //used to check the validity of footbridge links
    bool IsATrackElementAdjacentToLink(int Caller, int HLocIn, int VLocIn, int LinkIn); //true if there is an element adjacent to LinkIn
        //for element at HLoc & VLoc
    bool IsElementDefaultLength(int Caller, TTrackElement &TrackElement, bool FirstTrack, bool &LengthDifferent, bool &SpeedDifferent);
        //true if track at link positions [0] & [1] if FirstTrack true, else that at [2] & [3] in TrackElement has the default length
        //and speed limit, return true if so
    bool IsNamedNonStationLocationPresent(int Caller, int HLoc, int VLoc); //true if a non-station named location at HLoc & VLoc
    bool IsLCAtHV(int Caller, int HLoc, int VLoc); //true if a level crossing is found at H & V
    bool IsLCBarrierDownAtHV(int Caller, int HLoc, int VLoc); //true if an open (to trains) level crossing is found at H & V
    bool IsLCBarrierUpAtHV(int Caller, int HLoc, int VLoc); //true if a closed (to trains) level crossing is found at H & V
    bool IsLCBarrierFlashingAtHV(int Caller, int HLoc, int VLoc); //true if barrier is in process of opening or closing at H & V
    bool IsPlatformOrNamedNonStationLocationPresent(int Caller, int HLoc, int VLoc); //true if a non-station named location or platform
        //at HLoc & VLoc
    bool IsTrackLinked(int Caller); //true if track has been successfully linked (not used any more)
    bool LinkTrack(int Caller, bool &LocError, int &HLoc, int &VLoc, bool FinalCall); //attempt to link the track and return true if
        //successful, if unsuccessful return error flag and position of the first element that can't be linked together with an
        //appropriate message.  This is a link checking (FinalCall false) or linking (FinalCall true) function, with messages, called by
        //TryToConnectTrack, which handles linking and all other associated functions
    bool LinkTrackNoMessages(int Caller, bool FinalCall); //attempt to link the track and return true if successful, don't issue any
        //screen messages.  This is also a link checking (FinalCall false) or linking (FinalCall true) function, without messages,  that is
        //called by TryToConnectTrack, which handles linking and all other associated functions
    bool LocationNameAllocated(int Caller, AnsiString LocationName); //true if a non-empty LocationName found in LocationNameMultiMap
    bool LocationsNotNamed(int Caller); //true if there are unnamed NamedLocationElements (includes footbridges)
    bool MatchingPoint(int Caller, unsigned int TrackVectorPosition, unsigned int DivergingPosition); //true if the two vector positions
        //are points that have a straight and a diverging leg, are linked together by their diverging legs, and both are set either to
        //straight or to diverge
    bool NamedLocationElementAt(int Caller, int HLoc, int VLoc); //true if the active or inactive TrackElement at HLoc & VLoc has
        //its FixedNamedLocationElement member true
    bool NoActiveOrInactiveTrack(int Caller); //true if there is no active or inactive track in the railway
    bool NoActiveTrack(int Caller); //true if there is no active track in the railway
    bool NoGaps(int Caller); //true if there are no gaps
    bool NoNamedLocationElements(int Caller); //true if there are no NamedLocationElements (includes footbridges)
    bool NonFootbridgeNamedLocationExists(int Caller); //true if there is a platform, NamedNonStationLocation or Concourse present in
        //the railway
    bool OneNamedLocationElementAtLocation(int Caller, AnsiString LocationName); //true if there is at least one named location element
        //with name 'LocationName', used in timetable integrity checking
    bool OneNamedLocationLongEnoughForSplit(int Caller, AnsiString LocationName); //check sufficient track elements with same LocationName
        // linked together without any trailing point links to allow a train split.  Only one length is needed to return true, but this
        //doesn't mean that all platforms at the location are long enough.  When a split is required during operation a specific check is
        //made using ThisNamedLocationLongEnoughForSplit.  Need at least two linked track elements with the same LocationName, with connected
        //elements at each end, which may or may not be named and no connections via point trailing links.  Note that these conditions
        //exclude opposed buffers since these not linked.  Used in timetable integrity checking.
    bool OtherTrainOnTrack(int Caller, int NextPos, int NextEntryPos, int OwnTrainID); //true if another train on NextEntryPos track
        //of element at NextPos, whether bridge or not, return false if not, or if NextPos == -1, or if only own train on the track
    bool PlatformOnSignalSide(int Caller, int HLoc, int VLoc, int SpeedTag, Graphics::TBitmap* &SignalPlatformGraphic); //check whether
        //there is a platform present at HLoc & VLoc at the same side as the signal represented by SpeedTag, if so return true, and
        //also return a pointer to the appropriate platform graphic (same as a normal platform graphic but with a bit cut out for the
        //signal)
    bool RepositionAndMapTrack(int Caller); //when track is being built it is entered into the TrackVector in the order in which it is
        //built, and the TrackMap reflects that positioning.  When the track is linked, the vector is rebuilt in track element position
        //order, and the map is also rebuilt to reflect the new positions.  Called during track linking, returns true if successful.
    bool ResetConnClkCheckUnsetGapJumps(int Caller); //sets all Conns and CLks to -1 except for gapjumps that match and are properly set,
        //returns true for any unset gaps
    bool ResetGapsFromGapMap(int Caller); //called by RepositionAndMapTrack to reset the connecting elements of all set gaps (their
        //TrackVector positions will have changed during the repositioning process), returns true if successful
    bool ReturnNextInactiveTrackElement(int Caller, TTrackElement &Next); //return a reference to the inactive track element pointed
        //to by NextTrackElementPtr (during zoomed-in or out track rebuilding, or writing image files), return true if there is a next
        //one or false if not
    bool ReturnNextTrackElement(int Caller, TTrackElement &Next); //return a reference to the active track element pointed
        //to by NextTrackElementPtr (during zoomed-in or out track rebuilding, or writing image files), return true if there is a next
        //one or false if not
    bool ThisNamedLocationLongEnoughForSplit(int Caller, AnsiString LocationName, int FirstNamedElementPos, int &SecondNamedElementPos, int &FirstNamedLinkedElementPos, int &SecondNamedLinkedElementPos);
        //see above under 'OneNamedLocationLongEnoughForSplit'
    bool TimetabledLocationNameAllocated(int Caller, AnsiString LocationName); //true if a non-empty LocationName found as a timetabled location name
        //i.e. not as a continuation name
    bool TrainOnLink(int Caller, int HLoc, int VLoc, int Link, int &TrainID); //new at v1.2.0; checks whether a train present at input location and link and returns its ID if so
    bool TryToConnectTrack(int Caller, bool &LocError, int &HLoc, int &VLoc, bool GiveMessages); //handles all tasks associated with track
        //linking, returns true if successful (see also LinkTrack & LinkTrackNoMessages above)

    Graphics::TBitmap *GetFilletGraphic(int Caller, TTrackElement TrackElement); //return a pointer to the point fillet (the bit that
        //appears to move when points are changed) for the point and its Attribute specified in TrackElement
    Graphics::TBitmap *RetrieveStripedNamedLocationGraphicsWhereRelevant(int Caller, TTrackElement TrackElement); //return a pointer to
        //the striped (i.e. when unnamed) graphic corresponding to TrackElement, if TrackElement isn't a named element just return its
        //normal graphic

    int FindClosestLinkPosition(int Caller, int StartTVPosition, int EndTVPosition); //return the link array position for the element
        //at StartTVPosition that gives the closest link to the element at EndTVPosition. NB the StartTVPosition is expected to be a single
        //track element as only positions 0 & 1 are checked
    int GetAnyElementOppositeLinkPos(int Caller, int TrackVectorPosition, int LinkPos, bool &Derail); //return the opposite link position for
    //the element at TrackVectorPosition with link position LinkPos, if the element is points and they are set against the exit then
    //&Derail is returned true
    int GetTrackVectorPositionFromString(int Caller, AnsiString String, bool GiveMessages); //takes the ElementID value (an AnsiString)
        //(e.g. "8-13", "N43-N127", etc) and returns the corresponding track vector position, if none is found then -1 is returned
    int GetVectorPositionFromTrackMap(int Caller, int HLoc, int VLoc, bool &FoundFlag); //returns the track vector position corresponding
        //to the Hloc & VLoc positions, FoundFlag indicates whether an element is found or not, and if not -1 is returned
    int NumberOfGaps(int Caller); //returns the number of gaps in the railway

    TIMPair GetVectorPositionsFromInactiveTrackMap(int Caller, int HLoc, int VLoc, bool &FoundFlag); //similar to
        //GetVectorPositionFromTrackMap but for inactive elements, a pair is returned because there can be up to 2 platforms at a specific
        //position

    TLocationNameMultiMapIterator FindNamedElementInLocationNameMultiMap(int Caller, AnsiString LocationName, TTrackVectorIterator TrackElement, AnsiString &ErrorString);
        //searches LocationNameMultiMap to check if the element pointed to by the TTrackVectorIterator has the name LocationName.  If it
        //finds it the pointer TLocationNameMultiMapIterator is returned.  If it fails ErrorString is set to an appropriate text to allow
        //the calling function to report the error.  Otherwise it is set to "".

    TTrackElement &GetInactiveTrackElementFromTrackMap(int Caller, int HLoc, int VLoc); //return a reference to the inactive element at HLoc & VLoc, if
        //no element is found an error is thrown
    TTrackElement &GetTrackElementFromTrackMap(int Caller, int HLoc, int VLoc); //return a reference to the element at HLoc & VLoc, if
        //no element is found an error is thrown
    TTrackElement &InactiveTrackElementAt(int Caller, int At); //a range-checked version of InactiveTrackElement.at(At)
    TTrackElement &SelectVectorAt(int Caller, int At); //a range-checked version of SelectVector.at(At)
    TTrackElement &TrackElementAt(int Caller, int At); //a range-checked version of TrackElement.at(At)

    TTrackVectorIterator GetTrackVectorIteratorFromNamePosition(int Caller, int Position); //takes an adjusted vector position value from
        //either vector (if active, Position = -TruePos -1, if inactive, Position = TruePos) and returns a pointer to the relevant element

    void AddName(int Caller, TTrackVectorIterator TrackElement, AnsiString Name); //TrackElement.LocationName becomes 'Name' (for active
        //and inactive elements) and, if TrackElement is a platform or named non-station location, any active element at the same
        //HLoc & VLoc position has its ActiveTrackElementName set to 'Name'.
    void BuildGapMapFromTrackVector(int Caller); //examine TrackVector and whenever find a new gap pair enter it into GapMap
    void CalcHLocMinEtc(int Caller); //examine TrackVector, InactiveTrackVector and TextVector, and set the values that indicate the
        //extent of the railway (HLocMin, VLocMin, HLocMax & VLocMax) for use in zoomed in and out displays and in saving railway images
    void ChangeLocationNameMultiMapEntry(int Caller, AnsiString NewName, TLocationNameMultiMapIterator SNIterator); //changes the
        //LocationName in the name multimap to NewName at the location pointed to by the TLocationNameMultiMapIterator from whatever it
        //was before.  Accepts null entries so that a formerly named element can have the name changed to "".
    void CheckGapMap(int Caller); //validity test
    void CheckLocationNameMultiMap(int Caller); //validity test
    void CheckMapAndInactiveTrack(int Caller); //validity test
    void CheckMapAndTrack(int Caller); //validity test
    void DecrementValuesInGapsAndTrackAndNameMaps(int Caller, unsigned int VecPos); //after an element has been erased from the
        //TrackVector, all the later elements are moved down one.  This function decrements the position values for all values above
        //that of the erased element in the gap elements, TrackMap and LocationNameMultiMap.
    void DecrementValuesInInactiveTrackAndNameMaps(int Caller, unsigned int VecPos); //after an element has been erased from the
        //InactiveTrackVector, all the later elements are moved down one.  This function decrements the position values for all values
        //above that of the erased element in both InactiveTrack2MultiMap and LocationNameMultiMap.
    void EnterLocationName(int Caller, AnsiString LocationName, bool AddingElements); //all platform, concourse, footbridge & non-station named location
        //elements are able to have a LocationName allocated, and track elements (including footbridges) are able to have an
        //ActiveTrackElementName allocated provided there is an adjacent platform or a NamedNonStationLocation.  To set these names the
        //user selects a single named location element (not a footbridge), enters the name, and this is then allocated as a LocationName
        //to all linked platform, concourse and footbridge elements, and as an ActiveTrackElementName to all track elements adjacent to
        //platforms (inc footbridge tracks if (but only if) they have a platform at that location).
    void EraseLocationAndActiveTrackElementNames(int Caller, AnsiString LocationName); //examines LocationNameMultiMap and if the
        //LocationName is found all elements at that H & V (in both active and inactive vectors) have the name erased both as a
        //LocationName and a ActiveTrackElementName.  The LocationNameMultiMap is also rebuilt to correspond to the new names in the
        //vectors.
    void EraseTrackElement(int Caller, int HLocInput, int VLocInput, int &ErasedTrackVectorPosition, bool &TrackEraseSuccessfulFlag, bool InternalChecks);
        //erases all active and inactive track elements at HLocInput & VLocInput from the vectors, and, if any of these elements are named
        //the entries are erased from LocationNameMultiMap and the corresponding name is removed from the display and from all other
        //linked named elements
    void GetScreenPositionsFromTruePos(int Caller, int &ScreenPosH, int &ScreenPosV, int HPosTrue, int VPosTrue); //with large railways
        //only part of the railway is displayed on screen, and this function converts true (relative to the whole railway) H & V positions
        //to screen (relative to the displayed screen) H & V positions
    void GetTrackLocsFromScreenPos(int Caller, int &HLoc, int &VLoc, int ScreenPosH, int ScreenPosV); //converse of
        //GetScreenPositionsFromTruePos except that in this function HLoc & VLoc are expressed in track elements (i.e. 16x16 pixel squares)
        //rather than in single pixels
    void GetTruePositionsFromScreenPos(int Caller, int &HPos, int &VPos, int ScreenPosH, int ScreenPosV); //converse of
        //GetScreenPositionsFromTruePos
    void LengthMarker(int Caller, TDisplay *Disp); //examine all elements in the TrackVector and if have a valid length mark the relevant
        //track using MarkOneLength
    void LoadBarriersDownVector(int Caller, std::ifstream &VecFile); // load all BarriersDownVector values from SessionFile
    void LoadOldTrack(int Caller, std::ifstream &VecFile); //version of LoadTrack prior to its being updated, used when changes made to
        //LoadTrack in order to allow existing saved railways to be loaded prior to resaving in the new format compatible with the new
        //LoadTrack
    void LoadTrack(int Caller, std::ifstream &VecFile); //load track elements (active & inactive) from the file into the relevant vectors
        //and maps, and try to link the resulting track
    void MarkOneLength(int Caller, TTrackElement TE, bool FirstTrack, TDisplay *Disp); //mark on screen a track element according to its
        //length and speed limit if either of these differ from their default values
    void PlotAndAddTrackElement(int Caller, int CurrentTag, int HLocInput, int VLocInput, bool &TrackPlottedFlag, bool InternalChecks);
        //called during track building or pasting, when an element identified by CurrentTag (i.e. its SpeedTag value) is to be placed at
        //position HLocInput & VLocInput.  If the element can be placed it is displayed and added to the relevant vector, and if named its
        //name is added to LocationNameMultiMap
    void PlotLCBaseElementsOnly(int Caller, TBarrierState State, int BaseElementSpeedTag, int HLoc, int VLoc, bool ConsecSignals, TDisplay *Disp); //just replot the basic track
        //elements at a level crossing (for flashing)
    void PlotLoweredLinkedLevelCrossingBarriers(int Caller, int BaseElementSpeedTag, int HLoc, int VLoc, bool ConsecSignals, TDisplay *Disp);
        //plot & open (to trains) all level crossings linked to TrackElement
    void PlotPlainLoweredLinkedLevelCrossingBarriersAndSetMarkers(int Caller, int BaseElementSpeedTag, int HLoc, int VLoc, TDisplay *Disp);//plot
        //LC elements without any base elements, and set TempMarker true - used in ClearandRebuildRailway
    void PlotPlainRaisedLinkedLevelCrossingBarriersAndSetMarkers(int Caller, int BaseElementSpeedTag, int HLoc, int VLoc, TDisplay *Disp);//plot
        //LC elements without any base elements, and set TempMarker true - used in ClearandRebuildRailway
    void PlotRaisedLinkedLevelCrossingBarriers(int Caller, int BaseElementSpeedTag, int HLoc, int VLoc, TDisplay *Disp); //plot & close (to trains) all level
        //crossings linked to TrackElement
    void PlotGap(int Caller, TTrackElement TrackElement, TDisplay *Disp); //plots a gap on screen - may be set or unset
    void PlotPoints(int Caller, TTrackElement TrackElement, TDisplay *Disp, bool BothFillets); //plot points on screen according to how
        //they are set (Attribute value), or, with both fillets if BothFillets is true (the fillet is the bit that appears to move when
        //points are changed)
    void PlotSignal(int Caller, TTrackElement TrackElement, TDisplay *Disp); //plot signals on screen according to their aspect (Attribute
        //value)
    void PlotSmallRailway(int Caller, TDisplay *Disp); //plot on screen the zoomed-out railway
    void PlotSmallRedGap(int Caller); //plot on screen in zoomed-out mode and in gap setting mode a small red square corresponding to
        //the gap position that is waiting to have its matching gap selected (see also ShowSelectedGap)
    void PopulateLCVector(int Caller); //add all LCs to LCVector - note that this contains all LC elements whether linked to others or not
    void RebuildLocationNameMultiMap(int Caller); //clears the existing LocationNameMultiMap and rebuilds it from TrackVector and
        //InactiveTrackVector.  Called after the track is linked as many of the vector positions are likely to change - called from
        //RepositionAndMapTrack(); after names are changed in EraseLocationAndActiveTrackElementNames; and after the name changes in
        //EnterLocationName.
    void RebuildTrack(int Caller, TDisplay *Disp, bool BothPointFilletsAndBasicLCs); //called by TInterface::ClearandRebuildRailway to replot
        //all the active and inactive track elements, BothPointFillets indicates whether points are to be plotted according to how they
        //are set - for operation, or with both fillets - when not operating or during development (the fillet is the bit that appears to
        //move when points are changed)
    void ResetAllTrainIDElements(int Caller); //track elements have members that indicates whether and on what track a train is present
        //(TrainIDOnElement, TrainIDOnBridgeTrackPos01 and TrainIDOnBridgeTrackPos23).  This function resets them all to their
        //non-train-present state of -1.  Called by TTrainController::UnplotTrains
    void ResetAnyNonMatchingGaps(int Caller); //called by EraseTrackElement after the element has been erased and the vector positions
        //changed, in order to reset a matching gaps if the erased element was a set gap
    void ResetLevelCrossings(int Caller); //set all LC attributes to 0 (closed to trains)
    void ResetPoints(int Caller); //called on exit from operation to reset all points to non-diverging or to left fork (Attribute = 0)
    void ResetSignals(int Caller); //called on exit from operation to reset all signals to red (Attribute = 0)
    void SaveChangingLCVector(int Caller, std::ofstream &OutFile); ////save all changing vector values (used for error file)
    void SaveSessionBarriersDownVector(int Caller, std::ofstream &OutFile); //save all vector values to the session file
    void SaveTrack(int Caller, std::ofstream& VecFile); //save all active and inactive track elements to VecFile
    void SearchForAndUpdateLocationName(int Caller, int HLoc, int VLoc, int SpeedTag); //checks all locations that are adjacent to the
        //one entered for linked named location elements, and if any LocationName is found in any of the linked elements that name is
        //used for all elements that are now linked to it.  The location entered doesn't need to be a FixedNamedLocationElement and there
        //doesn't even need to be an element there.  Used during EraseTrackElement (in which case the SpeedTag is that for the element
        //that is erased) and PlotAndAddTrackElement, to bring the named location and timetable naming up to date with the deletion or
        //insertion.
    void SetAllDefaultLengthsAndSpeedLimits(int Caller); //Work through all elements in TrackVector setting all lengths & speed limits
        //to default values - including both tracks for 2-track elements
    void SetElementID(int Caller, TTrackElement &TrackElement); //convert the position values for the TrackElement into an identification
        //string and load in ElementID
    void SetLCAttributeAtHV(int Caller, int HLoc, int VLoc, int Attr); //set LC attribute at H & V; 0=closed to trains, 1 = open to trains, 2 = changing state = closed to trains
    void SetLinkedLevelCrossingBarrierAttributes(int Caller, int HLoc, int VLoc, int Attr);//set linked LC attributes; 0=closed to trains, 1 = open to trains, 2 = changing state = closed to trains
    void SetStationEntryStopLinkPosses(int Caller); //called when trying to link track and when a name changed when track already linked.
        //Examines all track elements that have ActiveTrackElementName set, sums the number of consecutive elements with the same name,
        //and sets the EntryLink values for the front of train stop points for each direction.  For stations (not non-station named
        //locations) of length n, where n > 1, stop element is [floor((n+1)/2) + 1] from each end (unless buffers at one or both ends in
        //which case stop points are the end elements).  Note that for a single element the stop point is the element itself (formula
        //doesn't apply).  For NamedNonStationLocations the stop points are at the end elements to allow trains to stack up.
    void ShowSelectedGap(int Caller, TDisplay *Disp); //called during gap setting to mark a gap with a red circle - after which the
        //program awaits user selection of the matching gap
    void TrackClear(int Caller); //empty the track and inactive track vectors, the corresponding track maps, and LocationNameMultiMap
    void TrackPush(int Caller, TTrackElement TrackElement); //insert TrackElement into the relevant vector and map, and, if named, insert
        //the name in LocationNameMultiMap
    void WriteOperatingTrackToImage(int Caller, Graphics::TBitmap *Bitmap); //called by TInterface::SaveOperatingImage1Click to add all
        //track element graphics to the image file in their operating state
    void WriteTrackToImage(int Caller, Graphics::TBitmap *Bitmap); //called by TInterface::SaveImageNoGrid1Click,
        //TInterface::SaveImageAndGrid1Click and TInterface::SaveImageAndPrefDirs1Click to add all track element graphics to the image
        //file in their non-operating state

    TTrack(); //constructor, only one object of this class
    ~TTrack(); //destructor
    };

//---------------------------------------------------------------------------

extern TTrack *Track; //the object pointer, object created in InterfaceUnit

//---------------------------------------------------------------------------
//PrefDir & Route functions
//---------------------------------------------------------------------------

enum TTruncateReturnType {NotInRoute, InRouteTrue, InRouteFalse}; //a flag used during route truncation to indicate the nature of the
    //selected element, it could be not in a route (NotInRoute), in a route but invalid (InRouteFalse), or in a route and valid
    //(InRouteTrue)

enum TPrefDirRoute {PrefDirCall, RouteCall}; //used in TOnePrefDir::PrefDirMarker to indicate whether the function is being called for a
    //preferred direction (PrefDirCall) or a route (RouteCall)

//---------------------------------------------------------------------------

class TOnePrefDir  //the basic preferred direction class, consisting of any number of elements with preferred directions set.  Used during
    //setting up preferred directions and track lengths (ConstructPrefDir), and for all completed preferred directions in the railway
    //(EveryPrefDir)
{
private://don't want descendant (TOneRoute) to access the PrefDir4MultiMap

typedef std::multimap<THVPair, unsigned int, TMapComp> TPrefDir4MultiMap; //HLoc&VLoc as a pair, and PrefDirVectorPosition, can be up to
    //4 values at any H&V
typedef std::multimap<THVPair, unsigned int, TMapComp>::iterator TPrefDir4MultiMapIterator;
typedef std::pair<THVPair, unsigned int> TPrefDir4MultiMapEntry;

TPrefDir4MultiMap PrefDir4MultiMap; //the pref dir multimap - up to 4 values (up to 2 tracks per element each with 2 directions)

//inline functions
void ClearPrefDir() {PrefDirVector.clear(); SearchVector.clear(); PrefDir4MultiMap.clear();} //empty the existing vectors & map

//functions defined in .cpp file
int GetOnePrefDirPosition(int Caller, int HLoc, int VLoc); //although there may be up to four entries at one H & V position this
    //function gets just one. It is used in EraseFromPrefDirVectorAnd4MultiMap by being called as many times as there are PrefDir
    //elements at H & V.
TPrefDir4MultiMapIterator GetExactMatchFrom4MultiMap(int Caller, unsigned int PrefDirVectorPosition, bool &FoundFlag); //retrieves a
    //PrefDir4MultiMap iterator to the PrefDir element at PrefDirVectorPosition.  Used during ErasePrefDirElementAt to erase the
    //relevant element in the multimap.  If nothing is found this is an error but the error message is given in the calling function.
void StorePrefDirElement(int Caller, TPrefDirElement LoadPrefDirElement); //store a single pref dir element in the vector & map
void ErasePrefDirElementAt(int Caller, int PrefDirVectorPosition); //erase a single element from PrefDirVector and 4MultiMap,
    //decrementing the remaining PrefDirElementNumbers in 4MultiMap if they are greater than the erased value.
void CheckPrefDir4MultiMap(int Caller); //diagnostic validity check
void DecrementPrefDirElementNumbersInPrefDir4MultiMap(int Caller, unsigned int ErasedElementNumber); //called after ErasePrefDirElementAt
    // to decrement the remaining PrefDirElementNumbers in 4MultiMap if they are greater than the erased value.

protected: //descendant (TOneRoute) can access these

typedef std::vector<TPrefDirElement> TPrefDirVector; //the pref dir vector type
typedef std::vector<TPrefDirElement>::iterator TPrefDirVectorIterator;
typedef std::vector<TPrefDirElement>::const_iterator TPrefDirVectorConstIterator;

static const int PrefDirSearchLimit = 30000; //limit to the number of elements searched in attempting to find a preferred direction

//[dropped as not a good strategy because gaps interfered with direct line searches - instead introduced TotalSearchCount and now use that
    //to limit searches. Leave in though in case rethink strategy later on]  Search limit values - set the H&V limits when searching for
    //the next pref dir element (or route as inherited by TOneRoute), all points on search path must lie within 15 elements greater than
    //the box of which the line between start and finish is a diagonal (else search takes too long)
int SearchLimitLowH;
int SearchLimitHighH;
int SearchLimitLowV;
int SearchLimitHighV;
int TotalSearchCount; //counts search elements, used to abort searches (prefdirs or routes) if reaches too high a value

TPrefDirVector PrefDirVector, SearchVector; //pref dir vectors, first is the main vector, second used to store search elements
    //temporarily

//functions defined in .cpp file
bool PresetAutoRouteElementValid(int Caller, TPrefDirElement ElementIn, int EntryPos); //added at v1.2.0
    //Checks ElementIn and returns true only if a single prefdir set at that H&V, with EntryPos giving entry position, not points,
    //crossovers, signals with wrong direction set, or buffers.
bool SearchForPrefDir(int Caller, TTrackElement TrackElement, int XLinkPos, int RequiredPosition); //try to find a selected element from
    //a given start position.  Enter with CurrentTrackElement stored in the PrefDirVector, XLinkPos set to the link to search on, &
    //SearchVector cleared unless entered recursively.  Function is a continuous loop that exits when find required element (returns true)
    //or reaches a buffer or continuation or otherwise fails a search condition (returns false).
void ConvertPrefDirSearchVector(int Caller); //called after a successful search to add the elements from the search vector to the
    //pref dir vector

public:

//inline functions
unsigned int PrefDirSize() const {return PrefDirVector.size();} //return the vector size
unsigned int SearchVectorSize() const {return SearchVector.size();} //as above
void ExternalClearPrefDirAnd4MultiMap() {ClearPrefDir();} //empty the existing preferred direction vector & map - for use by other classes

//functions defined in .cpp file
bool CheckOnePrefDir(int Caller, int NumberOfActiveElements, std::ifstream &VecFile); //Called before PrefDir loading as part of the
    //FileIntegrityCheck function in case there is an error in the file.  Very similar to LoadPrefDir but with value checks instead
    //of storage in PrefDirVector.
bool EndPossible(int Caller, bool &LeadingPoints); //used when setting preferred directions, true if able to finish at the last selected
    //element (can't finish if there is only one element or if end on leading points)
bool GetNextPrefDirElement(int Caller, int HLoc, int VLoc, bool &FinishElement); //used when continuing a chain of preferred directions
    //or element lengths. Tries to find a set of linked tracks between the last selected element and the one at HLoc & VLoc, and returns
    //true if it finds one.  FinishElement is returned true if the element selected is a buffer or continuation - in which case the chain
    //is complete
bool GetStartAndEndPrefDirElements(int Caller, TPrefDirElement &StartElement, TPrefDirElement &EndElement, int &LastIteratorValue); //Called when searching for
    //start and end PrefDirElements when setting up automatic signals routes in PreStart mode
bool GetPrefDirStartElement(int Caller, int HLoc, int VLoc); //used when beginning a chain of preferred directions or element lengths.
    //Enter with HLoc & VLoc set to selected element & check if selected element is a valid track element, return false if not, if it
    //is store it as the first entry in PrefDirVector and return true
bool GetPrefDirTruncateElement(int Caller, int HLoc, int VLoc); //called during PrefDir build or distance setting. It truncates at &
    //including the first element in the PrefDir vector that matches H & V.  After the truncate the final element of the remaining
    //PrefDir has its data members reset to the same defaults as would be the case if the PrefDir had been built up to that point - i.e.
    //for first element or a leading point.
bool ValidatePrefDir(int Caller); //checks that all elements in PrefDirVector have been properly set, i.e. don't have their default
    //values, and that every element is connected to the next element
int LastElementNumber(int Caller) const; //return the vector position of the last element in the vector (i.e. one less than the vector
    //size)
TPrefDirVectorIterator LastElementPtr(int Caller); //return a pointer to the last element in the vector
const TPrefDirElement &GetFixedPrefDirElementAt(int Caller, int At) const; //return a non-modifiable element at PrefDirVector position 'At'
TPrefDirElement &GetModifiablePrefDirElementAt(int Caller, int At); //return a modifiable element at PrefDirVector position 'At'
const TPrefDirElement &GetFixedSearchElementAt(int Caller, int At) const; //return a non-modifiable element at SearchVector position 'At'
TPrefDirElement &GetModifiableSearchElementAt(int Caller, int At); //return a modifiable element at SearchVector position 'At'
void CalcDistanceAndSpeed(int Caller, int &OverallDistance, int &OverallSpeedLimit, bool &LeadingPointsAtLastElement); //used when setting
    //element lengths, returns in &OverallDistance the overall distance for the selected chain of elements and also the speed limit in
    //&OverallSpeedLimit, which is set to -1 if the speed limits vary over the chain
void ExternalStorePrefDirElement(int Caller, TPrefDirElement LoadPrefDirElement) {StorePrefDirElement(6, LoadPrefDirElement);} //store a
    //single pref dir element in the vector & map - used by other classes
void GetVectorPositionsFromPrefDir4MultiMap(int Caller, int HLoc, int VLoc, bool &FoundFlag, //return up to 4 vector positions for a given
        int &PrefDirPos0, int &PrefDirPos1, int &PrefDirPos2, int &PrefDirPos3);             //HLoc & VLoc; unused values return -1
void LoadOldPrefDir(int Caller, std::ifstream &VecFile); //old version of LoadPrefDir, used during development when the save format
    //changed so the old files could be loaded prior to resaving in the new format
void LoadPrefDir(int Caller, std::ifstream &VecFile); //load a vector and map of preferred directions from the file
void PrefDirMarker(int Caller, TPrefDirRoute PrefDirRoute, bool BuildingPrefDir, TDisplay *Disp) const; //PrefDir and route track marker,
    //including direction markers.  Function used for both PrefDirs (PrefDirRoute == PrefDirCall) and routes (PrefDirRoute == RouteCall).
    //The graphics for marker colours and direction are already stored in all PrefDirElements in TOnePrefDir and TOneRoute, and this
    //function is called to display them, all in the case of a PrefDir, but for a route only the first and last elements have direction
    //markers. No markers are displayed if a train is present on an element.  Also no display if EXGraphicPtr not set.  If building a
    //PrefDir (BuildingPrefDir true) then the start and end rectangles are also displayed.
void SavePrefDirVector(int Caller, std::ofstream &VecFile); //save the preferred direction vector to a file
void SaveSearchVector(int Caller, std::ofstream &VecFile); //save the search vector to a file
void WritePrefDirToImage(int Caller, Graphics::TBitmap *Bitmap); //used when creating a bitmap image to display preferred directions
    //(as on screen during 'Set preferred direction' mode)

//EveryPrefDir (declared in InterfaceUnit.h) functions (all external)
bool CheckPrefDirAgainstTrackVectorNoMessage(int Caller); //check loaded PrefDir against loaded track, and if discrepancies found clear
    //EveryPrefDir & PrefDir4MultiMap, messages are given by the calling routine.  Return true for OK
void CheckPrefDirAgainstTrackVector(int Caller); //check loaded PrefDir against loaded track, and if discrepancies found give message &
    //clear EveryPrefDir & PrefDir4MultiMap.
void ConsolidatePrefDirs(int Caller, TOnePrefDir *InputPrefDir); //used when a preferred direction has been set to add all the elements
    //to EveryPrefDir, except when they already exist in EveryPrefDir
void EraseFromPrefDirVectorAnd4MultiMap(int Caller, int HLoc, int VLoc); //erase element at HLoc and VLoc from the PrefDirVector and from
    //the 4MultiMap.  Note that this entails erasing up to four elements (2 directions and 2 tracks for 4-entry elements).
void EveryPrefDirMarker(int Caller, TDisplay *Disp); //Similar to PrefDirMarker but used only to mark EveryPrefDir - red for unidirectional
    //PrefDir & green for bidirectional. Colours taken from the route colours. Plot red first so green overwrites for bidirectional points.
void RealignAfterTrackErase(int Caller, int ErasedTrackVectorPosition); //after a track element is erased the preferred direction
    //elements are likely to be affected. This function erases any preferred direction elements that either correspond to the erased track
    //element, or were linked to it
void RebuildPrefDirVector(int Caller); //called after the track vector has been rebuilt following linking, to rebuild the preferred
    //direction vector to correspond to the element positions in the rebuilt track vector. Doesn't affect the preferred direction
    //multimap.
};

//---------------------------------------------------------------------------

class TOneRoute : public TOnePrefDir //a descendent of TOnePrefDir used for routes.  Used during contruction of a route (ConstructRoute)
    //and also for all completed routes, when each route is saved as an entry in the AllRoutesVector (see TAllRoutes)
{
public:

class TRouteFlashElement //a single flashing element of a route that flashes during setting
    {
    public:
    int HLoc, VLoc, TrackVectorPosition; //element values
    Graphics::TBitmap *OriginalGraphic, *OverlayGraphic; //the two graphics, non route-coloured and route-coloured respectively, these are
        //displayed alternately during flashing
    };

class TRouteFlash //the flashing route
    {
    public:
    std::vector<TRouteFlashElement> RouteFlashVector; //vector of TRouteFlashElements
    bool OverlayPlotted; //flag indicating the graphic that is currently displayed, true for the overlay (route-coloured)

    //both external
    void PlotRouteOverlay(int Caller); //display the overlay (route-coloured) graphic
    void PlotRouteOriginal(int Caller); //display the original (non route-coloured) graphic
    };

static const int RouteSearchLimit = 30000; //limit to the number of elements searched in attempting to find a route

IDInt ReqPosRouteID; //the route ID number of the route that is being extended backwards during route building, not needed for
    //session saves as routes in build are not saved in sessions
IDInt StartSelectionRouteID; //the route ID number of the route that is being extended forwards during route building, not
    //needed for session saves as routes in build are not saved in sessions

int RouteID; //the ID number of the route, this is needed for session saves
int StartRoutePosition; //TrackVectorPosition of the StartElement(s) set when the starting position of a new route is selected, note that
    //although there may be two StartElements (as there can be two preferred directions on a single element), there is only one
    //TrackVectorPosition as the element is the same for both
TPrefDirElement StartElement1, StartElement2; //the two preferred direction elements corresponding to the starting position of a new route
TRouteFlash RouteFlash; //the class member that allows the route to flash during setting up (see TRouteFlash above)

//inline functions
void ClearRoute() {PrefDirVector.clear(); SearchVector.clear();} //empty the route of any stored elements
void EraseRouteElementAt(TPrefDirElement *RouteElementPtr) {PrefDirVector.erase(RouteElementPtr);} //erase a single route element
void StoreRouteElementInPrefDirVector(TPrefDirElement LoadPrefDirElement) {LoadPrefDirElement.IsARoute = true; //store a single route
    PrefDirVector.push_back(LoadPrefDirElement);} //element in the PrefDirVector

//functions defined in .cpp file
bool FindForwardTargetSignalAttribute(int Caller, int &NextForwardLinkedRouteNumber, int &Attribute) const; //used when setting signal
    //aspects for a route by working forwards through the route to see what the next forward signal aspects is, because this determines
    //all the rearward signal aspects.
bool GetNonPreferredRouteStartElement(int Caller, int HLoc, int VLoc, bool ConsecSignalsRoute, bool Callon); //set the starting conditions for a non-
    //preferred (i.e. unrestricted) route selection beginning on HLoc & VLoc
bool GetNextNonPreferredRouteElement(int Caller, int HLoc, int VLoc, bool ConsecSignalsRoute, bool Callon, IDInt &ReqPosRouteID, bool &PointsChanged);
    //try to find a set of linked tracks between the route start element and the one at HLoc & VLoc.  If find one return true, set
    //&PointsChanged to true if any points need to be changed and &ReqPosRouteID to the route ID of the existing route to attach to,
    //if there is one, and -1 if not
bool GetPreferredRouteStartElement(int Caller, int HLoc, int VLoc, TOnePrefDir *EveryPrefDir, bool ConsecSignalsRoute, bool AutoSigsFlag);
    //set the starting conditions for a preferred direction or automatic signal route selection beginning on HLoc & VLoc
bool GetNextPreferredRouteElement(int Caller, int HLoc, int VLoc, TOnePrefDir *EveryPrefDir, bool ConsecSignalsRoute, bool AutoSigsFlag,
    IDInt &ReqPosRouteID, bool &PointsChanged); //try to find a set of linked tracks that lie on preferred directions between the route
    //start element and the one at HLoc & VLoc.  If find one return true, set &PointsChanged to true if any points need to be changed and
    //&ReqPosRouteID to the route ID of the existing route to attach to, if there is one, and -1 if not
bool PointsToBeChanged(int Caller) const; //called by GetNextNonPreferredRouteElement and GetNextPreferredRouteElement to check whether
    //or not any points on the selected route need to be changed
bool SearchForNonPreferredRoute(int Caller, TTrackElement CurrentTrackElement, int XLinkPos, int RequiredPosition, IDInt ReqPosRouteID);
    //called by GetNextNonPreferredRouteElement to carry out the search for linked track, and also called recursively
bool SearchForPreferredRoute(int Caller, TPrefDirElement PrefDirElement, int XLinkPos, int RequiredPosition, IDInt ReqPosRouteID,
    TOnePrefDir *EveryPrefDir, bool ConsecSignalsRoute, int EndSelectPosition, bool AutoSigsFlag); //called by
    //GetNextPreferredRouteElement to carry out the search for a valid route, and also called recursively
bool SetRearwardsSignalsReturnFalseForTrain(int Caller, int &Attribute, int PrefDirVectorStartPosition) const; //called by
    //TAllRoutes::SetAllRearwardsSignals to set rearwards signals from a specified starting position.  If a train is found during the
    //rearwards search then this function flags the fact so that the calling function can change its behaviour with respect to further
    //rearwards signal aspects.
void ConvertAndAddNonPreferredRouteSearchVector(int Caller, IDInt ReqPosRouteID); //called after a non-preferred (i.e. unrestricted) route
    //has been selected and has finished flashing, to add it to the AllRoutesVector
void ConvertAndAddPreferredRouteSearchVector(int Caller, IDInt ReqPosRouteID, bool AutoSigsFlag); //called after a preferred
    //(i.e. preferred direction or automatic signals) route has been selected and has finished flashing, to add it to the AllRoutesVector
void ForceCancelRoute(int Caller); //cancel a route immediately if a train occupies it when travelling in the wrong direction (or occupies
    //a crossover on a non-route line when the other track is in a route)
void GetRouteTruncateElement(int Caller, int HLoc, int VLoc, bool ConsecSignalsRoute, TTruncateReturnType &ReturnFlag); //Examines the
    //route to see whether the element at H & V is in the route, and if not returns a ReturnFlag value of NotInRoute.
    //If it is in a route but the element selected is invalid, then a message is given and returns with a ReturnFlag value of
    //InRouteFalse.  Otherwise the route is truncated at and including the element that matches H & V with a ReturnFlag value of
    //InRouteTrue.  Selection invalid if a train at or before the truncate point; select a bridge; trying to leave a single element; last
    //element to be left not a signal (for ConsecSignalsRoute or has AutoSigsFlag set); last element to be left a bridge, points or crossover
    //(for not ConsecSignalsRoute & AutoSigsFlag not set), or part of route locked.
void RouteImageMarker(int Caller, Graphics::TBitmap *Bitmap) const; //used when creating a bitmap image to display the route colours and
    //direction arrows (as on screen during operation) for an operating railway
void SetLCChangeValues(int Caller, bool ConsecSignalsRoute); //after a route has been selected successfully this function sets all LC change values
    //appropriately for the selected route type and location
void SetRemainingSearchVectorValues(int Caller); //called when setting unrestricted routes to set the route element values appropriately
    //after a successful search has been conducted.  It isn't needed for preferred routes because the element values are obtained from the
    //already set preferred direction elements
void SetRouteFlashValues(int Caller, bool AutoSigsFlag, bool ConsecSignalsRoute); //after a route has been selected successfully this function sets all
    //RouteFlash (see above) values appropriately for the selected route type and location
void SetRouteSearchVectorGraphics(int Caller, bool AutoSigsFlag, bool ConsecSignalsRoute); //set values for EXGraphicPtr and
    //EntryDirectionGraphicPtr for all elements in SearchVector so that the route displays with the correct colour
void SetRoutePoints(int Caller) const; //called when setting a route to set all points appropriately
void SetRouteSignals(int Caller) const; //called when setting a route to set all points appropriately.  Also called when a new
    //train is added at a position where a route has been set, when it is necessary to set the next rearwards signal to red, the next
    //yellow etc
};

//---------------------------------------------------------------------------

class TAllRoutes //the class that handles data and functions relating to all routes on the railway
{
public:

class TLockedRouteClass //handles routes that are locked because of approaching trains
    {
    public:
    int RouteNumber; //the vector position number of the relevant route in AllRoutesVector
    unsigned int TruncateTrackVectorPosition; //the TrackVector position of the element selected for truncation
    unsigned int LastTrackVectorPosition; //the TrackVector position of the last (i.e. most forward) element in the route
    int LastXLinkPos; //the XLinkPos value of the last (i.e. most forward) element in the route
    TDateTime LockStartTime; //the timetable time at which the route is locked, to start the 2 minute clock
    };

enum TRouteType {NoRoute, NotAutoSigsRoute, AutoSigsRoute} RouteType; //distinguishes between automatic signals routes and other types,
    //or no route at all (where this is used there is no need to distinguish between preferred direction and unrestricted routes)

typedef std::vector<TOneRoute> TAllRoutesVector; //the vector class that holds all the railway routes
typedef std::vector<TOneRoute>::iterator TAllRoutesVectorIterator;

typedef std::vector<TLockedRouteClass> TLockedRouteVector; //the vector class that holds all locked routes
typedef std::vector<TLockedRouteClass>::iterator TLockedRouteVectorIterator;

typedef std::pair<int, unsigned int> TRouteElementPair; //defines a specific element in a route, the first (int) value is the vector
    //position in the AllRoutesVector, and the second (unsigned int) value is the vector position of the element in the route's
    //PrefDirVector
typedef std::multimap<THVPair, TRouteElementPair, TMapComp> TRoute2MultiMap; //the multimap class holding the elements of all routes in the
    //railway.  The first entry is the HLoc & VLoc pair values of the route element, and the second is the TRouteElementPair defining the
    //element.  There are a maximum of 2 elements per HLoc & VLoc location
typedef TRoute2MultiMap::iterator TRoute2MultiMapIterator;
typedef std::pair<THVPair, TRouteElementPair> TRoute2MultiMapEntry;

class TCallonEntry //used to store relevant values when a call-on found, ready for plotting an unrestricted route
    {
    public:
    bool RouteOrPartRouteSet; //whether or not a route or part route already plotted
    int RouteStartPosition; //the stop signal trackvectorposition
    int PlatformPosition; //the first platform trackvectorposition

    TCallonEntry::TCallonEntry(bool RouteOrPartRouteSetIP, int RouteStartPositionIP, int PlatformPositionIP) {RouteOrPartRouteSet = RouteOrPartRouteSetIP;
            RouteStartPosition = RouteStartPositionIP; PlatformPosition = PlatformPositionIP;} //constructor
    };

std::vector<TCallonEntry> CallonVector; //the store of all call-on entries

bool LockedRouteFoundDuringRouteBuilding; //this flags the fact that a locked route has been found during route building in an
    //existing linked route which is erased prior to its elements being added to the new route

//the following variables store the locked route values for reinstating after a locked route has been found during route building in an
//existing linked route which is erased prior to its elements being added to the new route.  The locked route is erased in
//ClearRouteDuringRouteBuildingAt, and is reinstated in ConvertAndAddPreferredRouteSearchVector or
//ConvertAndAddNonPreferredRouteSearchVector.
int LockedRouteLastXLinkPos;
unsigned int LockedRouteTruncateTrackVectorPosition;
unsigned int LockedRouteLastTrackVectorPosition;
TDateTime LockedRouteLockStartTime;
//end of locked route values

bool RebuildRailwayFlag; //this is set whenever a route has to be cancelled forcibly in order to force a ClearandRebuildRailway at the
    //next clock tick if not in zoom-out mode to clear the now cancelled route on the display
bool RouteTruncateFlag; //used to flag the fact that a route is being truncated on order to change the behaviour of signal aspect
    //setting in SetRearwardsSignalsReturnFalseForTrain

const float LevelCrossingBarrierUpDelay; //the full value in seconds for which the level crossing flashes prior to closing to trains
const float LevelCrossingBarrierDownDelay; //the full value in seconds for which the level crossing flashes prior to opening to trains
const float PointsDelay; //the value in seconds for which points flash prior to being changed.  Used for the points flash period when
    //changing points manually and for the route flash period when points have to be changed
const float SignalsDelay; //the value in seconds for which signals flash prior to being changed.  Used for the route flash period when
    //points don't have to be changed
int NextRouteID; //stores the value for the route ID number that is next to be built
TAllRoutesVector AllRoutesVector; //the vector that stores all the routes on the railway
TLockedRouteVector LockedRouteVector; //the vector that stores all the locked routes on the railway
TOneRoute SignallerRemovedTrainAutoRoute; //if train was on an AutoSigsRoute when removed then this stores the route so that signals can be reset
TRoute2MultiMap Route2MultiMap; //the map that stores the elements of all routes on the railway (see TRoute2MultiMap for more info)

//inline functions
unsigned int AllRoutesSize() const {return AllRoutesVector.size();} //returns the number of routes in the railway
void AllRoutesClear() {AllRoutesVector.clear(); Route2MultiMap.clear();} //erases all routes from AllRoutesVector and from Route2MultiMap

//functions defined in .cpp file
bool CheckForLoopingRoute(int Caller, int EndPosition, int EndXLinkPos, int StartPosition);//return true if route loops back on itself
bool CheckRoutes(int Caller, int NumberOfActiveElements, std::ifstream &InFile); //performs an integrity check on the routes stored in a
    //session file and returns false if there is an error
bool DiagonalFouledByRouteOrTrain(int Caller, int HLoc, int VLoc, int DiagonalLinkNumber); //the track geometry allows diagonals to cross without
    //occupying the same track element, so when route plotting it is necessary to check if there is an existing route or a train on such a
    //crossing diagonal.  Returns true for a fouled (i.e. fouled by a route or a train) diagonal. New at v1.2.0
bool DiagonalFouledByRoute(int Caller, int HLoc, int VLoc, int DiagonalLinkNumber); //as above but only checks for a route (may or may not be a train
    //present (new at v1.2.0)
bool TAllRoutes::FindRouteNumberFromRoute2MultiMapNoErrors(int Caller, int HLoc, int VLoc, int ELink, int &RouteNumber); //if a route is present at
    //H, V & Elink returns true with RouteNumber giving vector position in AllRoutes vector.  Returns false for anything else including no element
    //or route at H & V etc. New at v1.2.0
bool GetAllRoutesTruncateElement(int Caller, int HLoc, int VLoc, bool ConsecSignalsRoute); //Examines all routes and for each uses
    //GetRouteTruncateElement to see if the element at H & V is present in that route.  The ReturnFlag value indicates InRouteTrue
    //(success), InRouteFalse (failure), or NotInRoute.  Messages are given in GetRouteTruncateElement.  If successful the route is
    //truncated at and including the element that matches H & V.  If ConsecSignalsRoute ensure only truncate to a signal, else prevent
    //truncation to a crossover, bridge or points, also prevent route being left less than 2 elements in length.
bool IsElementInLockedRouteGetPrefDirElementGetLockedVectorNumber(int Caller, int TrackVectorPosition, int XLinkPos,
    TPrefDirElement &PrefDirElement, int &LockedVectorNumber); //checks whether the preferred direction element at TrackVectorPosition
    //with XLinkPos value is in a locked route and returns true if so together with the element itself copied to &PrefDirElement & the
    //LockedRouteVector position in &LockedVectorNumber
bool IsThereARouteAtIDNumber(int Caller, IDInt RouteID); //returns true if there is a route with the given ID number - added at v1.3.1 (see function for details)
bool LoadRoutes(int Caller, std::ifstream &InFile); //loads the routes from a session file
bool RouteLockingRequired(int Caller, int RouteNumber, int RouteTruncatePosition); //route locking is required (returns true) if a moving train
    //is within 3 signals back from the RouteTruncatePosition (on the route itself or on any linked routes, or on the element immediately
    //before the start of the route or linked route - this because train cancels route elements that it touches) unless the first signal is
    //red, then OK
bool TrackIsInARoute(int Caller, int TrackVectorPosition, int LinkPos); //examines Route2MultiMap and if the element at
    //TrackVectorPosition with LinkPos (can be entry or exit) is found it returns true (for crossovers returns true whichever track the route is on),
    //else returns false.
int GetRouteVectorNumber(int Caller, IDInt RouteID); //returns a route's position in AllRoutesVector from its ID, throws an
    //error if a matching route isn't found
const TOneRoute &GetFixedRouteAt(int Caller, int At) const; //returns a constant reference to the route at AllRoutesVector position 'At',
    //after performing range checking on the 'At' value and throwing an error if out of range
const TOneRoute &GetFixedRouteAtIDNumber(int Caller, IDInt RouteID) const; //returns a constant reference to the route with ID number
    //RouteID.  If no route is found with that ID an error is thrown
TOneRoute &GetModifiableRouteAt(int Caller, int At); //returns a modifiable reference to the route at AllRoutesVector position 'At',
    //after performing range checking on the 'At' value and throwing an error if out of range
TOneRoute &GetModifiableRouteAtIDNumber(int Caller, IDInt RouteID); //returns a modifiable reference to the route with ID number
    //RouteID.  If no route is found with that ID an error is thrown
TRouteElementPair FindRoutePairFromRoute2MultiMap(int Caller, int HLoc, int VLoc, int ELink, //examines Route2MultiMap and returns a
    TRoute2MultiMapIterator &Route2MultiMapIterator); //TRouteElementPair if one is found with the passed values of H, V and ELink.
    //Also returned as a reference is an iterator to the found element in the map to assist in erasing it.  Called by
    //TAllRoutes::RemoveRouteElement.  Note that only need ELink (as well as H & V) to identify uniquely, since only bridges can have
    //two routes on them & their track ELinks are always different.  Messages are given for failure.
TRouteElementPair GetRouteElementDataFromRoute2MultiMap(int Caller, int HLoc, int VLoc, TRouteElementPair &SecondPair); //retrieve up to
    //two TRouteElementPair entries from Route2MultiMap at H & V, the first as a function return and the second in the reference
    //SecondPair.  If there's only one then it's the function return
TRouteType GetRouteTypeAndGraphics(int Caller, int TrackVectorPosition, int LinkPos, Graphics::TBitmap* &EXGraphicPtr,
    Graphics::TBitmap* &EntryDirectionGraphicPtr); //examines Route2MultiMap for the element at TrackVectorPosition with LinkPos
    //(can be entry or exit) and returns the appropriate route type - NoRoute, NotAutoSigsRoute, or AutoSigsRoute.  If element is in a
    //route then the EXGraphicPtr is returned, and if either the start or end of a route then the correct EntryDirectionGraphicPtr is
    //returned, else a transparent element is returned.  Function is used int TrainUnit for retaining AutoSigsRoutes but erasing others
    //after train passes, and for picking up the correct background graphics for replotting of AutoSigsRoutes; also used in
    //CallingOnAllowed
TRouteType GetRouteTypeAndNumber(int Caller, int TrackVectorPosition, int LinkPos, int &RouteNumber); //examines Route2MultiMap and if
    //the element at TrackVectorPosition with LinkPos (can be entry or exit) is found returns the appropriate route type - NoRoute,
    //NotAutoSigsRoute, or AutoSigsRoute and number (i.e. its position in AllRoutesVector).
void AddRouteElement(int Caller, int HLoc, int VLoc, int ELink, int RouteNumber, TPrefDirElement RouteElement); //a single
    //TPrefDirElement is added to both PrefDirVector (for the route at RouteNumber) and Route2MultiMap.  Called from
    //TAllRoutes::StoreOneRoute.  Note that the IsARoute boolean variable is set in StoreRouteElementInPrefDirVector since that catches
    //all route elements wherever created
void CheckMapAndRoutes(int Caller); //diagnostic function - checks equivalence for each route between entries in PrefDirVector and those
    //in Route2MultiMap, and also that the size of the multimap and the sum of the sizes of all PrefDirVectors is the same.  Throws an
    //error if there is a discrepancy.
void ClearRouteDuringRouteBuildingAt(int Caller, int RouteNumber); //when attaching a new route section to an existing route, it is
    //sometimes necessary to erase the original route and create a new composite route.  This function erases all elements in the route
    //at RouteNumber using TAllRoutes->RemoveRouteElement to clear elements from Route2MultiMap and from the PrefDirVector.  Since all
    //elements for the route are removed RemoveRouteElement also clears the Route from AllRoutesVector.  Route numbers are decremented in the
    //map for route numbers that are greater than the route number that is removed.  The LockedRouteVector as also searched and if any relate
    //to the route that has been cleared they are erased too, but the fact that one has been found is recorded so that it can be re-established
    //later.
void DecrementRouteElementNumbersInRoute2MultiMap(int Caller, int RouteNumber, unsigned int ErasedElementNumber); //After a route element
    //has been erased from the relevant PrefDirVector and from Route2MultiMap, this function examines all the remaining entries in
    //Route2MultiMap with the same RouteNumber as that for the erased element.  Where a RouteElementNumber exceeds that for the erased
    //element it is decremented.
void DecrementRouteNumbersInRoute2MultiMap(int Caller, int RouteNumber); //after a route has been erased from AllRoutesVector and its
    //entries from Route2MultiMap, this function examines all the remaining entries in Route2MultiMap to see if their RouteNumbers
    //exceed that for the erased route.  Where this is so the RouteNumber is decremented.
void MarkAllRoutes(int Caller, TDisplay *Disp); //calls PrefDirMarker for all routes, with RouteCall set to identify a route call, and
    //BuildingPrefDir false.
void RemoveRouteElement(int Caller, int HLoc, int VLoc, int ELink); //erases the route element from Route2MultiMap and from the
    //PrefDirVector.
void Route2MultiMapInsert(int Caller, int HLoc, int VLoc, int ELinkIn, int RouteNumber, unsigned int RouteElementNumber); //insert an
    //entry in Route2MultiMap.  Called by TAllRoutes::AddRouteElement.
void SaveRoutes(int Caller, std::ofstream &OutFile); //save railway route information to a session file or an error file
void SetAllRearwardsSignals(int Caller, int Attribute, int RouteNumber, int RouteStartPosition); //set rearwards signals from the
    //specified route starting position
void SetTrailingSignalsOnAutoSigsRoute(int Caller, int TrackVectorPosition, int XLinkPos); //enter with signal at TrackVectorElement
    //already set to red by the passing train.  Identify the route that the TrackVectorPosition is in, carry out validity checks, then
    //call SetAllRearwardsSignals to set signals in this route and all linked rearwards routes, unless find a train (a) in the current
    //route, in which case the signals behind it are set (and behind any other trains in the current route), but only within the current
    //route; or (b) in a linked rear route, in which case the function sets no further signals.
void SetTrailingSignalsOnContinuationRoute(int Caller, int RouteNumber, int AccessNumber); //this is called by the InterfaceUnit at
    //intervals based on entries in the ContinuationAutoSigVector in TrainController to set signals on the AutoSigsRoute to correspond to
    //a train having exited the route at a continuation, and passing further signals (outside the simulated railway).  Initially the last
    //passed signal will be red, then at the first call it will change to yellow and earlier signals will change accordingly, then double
    //yellow, then green.  There are only 3 calls in all for any given route, and the AccessNumber changes from 0 to 1 to 2 for successive
    //calls.
void StoreOneRoute(int Caller, TOneRoute *Route); //a new (empty apart from RouteID) TOneRoute is added to the AllRoutesVector, which,
    //since it is the last to be added, will have a RouteNumber of AllRoutesSize() - 1.  Then each element of the new route is added in
    //turn using AddRouteElement, which uses HLoc, VLoc, ELink and RouteNumber to provide the information necessary to insert it into
    //both PrefDirVector and Route2MultiMap.
void StoreOneRouteAfterSessionLoad(int Caller, TOneRoute *Route); //a new (empty apart from RouteID) TOneRoute is added to the
    //AllRoutesVector after a session load.  Very similar to StoreOneRoute but here the RoutID that is already in Route is used.
void WriteAllRoutesToImage(int Caller, Graphics::TBitmap *Bitmap); //calls RouteImageMarker for each route in turn to display the route
    //colours and direction arrows on the bitmap image (as on screen during operation) for an operating railway

TAllRoutes::TAllRoutes() : LevelCrossingBarrierUpDelay(10.0), LevelCrossingBarrierDownDelay(30.0), PointsDelay(2.5), SignalsDelay(0.5), RebuildRailwayFlag(false) {;} //constructor
};

//---------------------------------------------------------------------------

extern TAllRoutes *AllRoutes; //the object pointer, object created in InterfaceUnit

//---------------------------------------------------------------------------
#endif
