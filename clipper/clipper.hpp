/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  6.4.2                                                           *
* Date      :  27 February 2017                                                *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2017                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
* Computer graphics and geometric modeling: implementation and algorithms      *
* By Max K. Agoston                                                            *
* Springer; 1 edition (January 4, 2005)                                        *
* http://books.google.com/books?q=vatti+clipping+agoston                       *
*                                                                              *
* See also:                                                                    *
* "Polygon Offsetting by Computing Winding Numbers"                            *
* Paper no. DETC2005-85513 pp. 565-575                                         *
* ASME 2005 International Design Engineering Technical Conferences             *
* and Computers and Information in Engineering Conference (IDETC/CIE2005)      *
* September 24-28, 2005 , Long Beach, California, USA                          *
* http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf              *
*                                                                              *
*******************************************************************************/

#ifndef clipper_hpp
#define clipper_hpp

#define CLIPPER_VERSION "6.4.2"

//use_int32: When enabled 32bit ints are used instead of 64bit ints. This
//improve performance but coordinate values are limited to the range +/- 46340
//#define use_int32

//use_xyz: adds a Z member to IntPoint. Adds a minor cost to perfomance.
//#define use_xyz

//use_lines: Enables line clipping. Adds a very minor cost to performance.
#define use_lines
  
//use_deprecated: Enables temporary support for the obsolete functions
//#define use_deprecated  

#include <list>
#include <set>
#include <cstdlib>
#include <functional>
#include <queue>
#include <cstddef>
#include "limits.h"
#include "allocation_schemes.hpp"

namespace ClipperLib {

enum ClipType { ctIntersection, ctUnion, ctDifference, ctXor };
enum PolyType { ptSubject, ptClip };
//By far the most widely used winding rules for polygon filling are
//EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
//Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
//see http://glprogramming.com/red/chapter11.html
enum PolyFillType { pftEvenOdd, pftNonZero, pftPositive, pftNegative };

#ifdef use_int32
  typedef int cInt;
  static cInt const loRange = 0x7FFF;
  static cInt const hiRange = 0x7FFF;
#else
  typedef signed long long cInt;
  static cInt const loRange = 0x3FFFFFFF;
  static cInt const hiRange = 0x3FFFFFFFFFFFFFFFLL;
  typedef signed long long long64;     //used by Int128 class
  typedef unsigned long long ulong64;

#endif

struct IntPoint {
  cInt X;
  cInt Y;
#ifdef use_xyz
  cInt Z;
  IntPoint(cInt x = 0, cInt y = 0, cInt z = 0): X(x), Y(y), Z(z) {};
#else
  IntPoint(cInt x = 0, cInt y = 0): X(x), Y(y) {};
#endif

  friend inline bool operator== (const IntPoint& a, const IntPoint& b)
  {
    return a.X == b.X && a.Y == b.Y;
  }
  friend inline bool operator!= (const IntPoint& a, const IntPoint& b)
  {
    return a.X != b.X  || a.Y != b.Y; 
  }
};
//------------------------------------------------------------------------------

typedef std::vector< IntPoint > Path;
typedef std::vector< Path > Paths;

inline Path& operator <<(Path& poly, const IntPoint& p) {poly.push_back(p); return poly;}
inline Paths& operator <<(Paths& polys, const Path& p) {polys.push_back(p); return polys;}

std::ostream& operator <<(std::ostream &s, const IntPoint &p);
std::ostream& operator <<(std::ostream &s, const Path &p);
std::ostream& operator <<(std::ostream &s, const Paths &p);

struct DoublePoint
{
  double X;
  double Y;
  DoublePoint(double x = 0, double y = 0) : X(x), Y(y) {}
  DoublePoint(IntPoint ip) : X((double)ip.X), Y((double)ip.Y) {}
};
//------------------------------------------------------------------------------

#ifdef use_xyz
typedef void (*ZFillCallback)(IntPoint& e1bot, IntPoint& e1top, IntPoint& e2bot, IntPoint& e2top, IntPoint& pt);
#endif

enum InitOptions {ioReverseSolution = 1, ioStrictlySimple = 2, ioPreserveCollinear = 4};
enum JoinType {jtSquare, jtRound, jtMiter};
enum EndType {etClosedPolygon, etClosedLine, etOpenButt, etOpenSquare, etOpenRound};

/*ideally, for maximum flexibility, we would redefine most Clipper classes
as templated on the memory manager, but that would be a *very* big change to
the code, requiring either templates on user code all the way up the place
where memory management is decided, or some form of type erasure (and if we
are going to use type erasure, we may as well use virtual inheritance
for cleaner code). Let's keep it simpler, at the cost of flexibility...*/
#ifdef CLIPPER_USE_ARENA
#  define CLIPPER_MMANAGER  ArenaMemoryManager
#else
#  define CLIPPER_MMANAGER  SimpleMemoryManager
#endif

class Clipper;
class ClipperOffset;
class PolyNode;
typedef std::vector< PolyNode* > PolyNodes;

class PolyNode
{ 
public:
    PolyNode() : Parent(0), Index(0), m_IsOpen(false) {};
    virtual ~PolyNode() {};
    Path Contour;
    PolyNodes Childs;
    PolyNode* Parent;
    PolyNode* GetNext() const;
    bool IsHole() const;
    bool IsOpen() const;
    int ChildCount() const;
private:
    JoinType m_jointype;
    EndType m_endtype;
    PolyNode* GetNextSiblingUp() const;
    //PolyNode& operator =(PolyNode& other); 
    unsigned Index; //node index in Parent.Childs
    bool m_IsOpen;
    void AddChild(PolyNode& child);
    friend class Clipper; //to access Index
    friend class ClipperOffset; 
};

/* IMPORTANT DIFFERENCE WITH ORIGINAL CLIPPER CODE:
   In the original ClipperLib, PolyTree was intended as another datatype to be instantiated at
   any time just as Paths. After refactoring the code to use custom memory managers, this is
   no longer possible, because PolyTree's internals contain references to memory areas that
   are allocated with the custom manager, so they cannot be used after calling manager.reset().
   For simplicity, the following usage pattern was decided:
       -Both Clipper.Clear() and ClipperOffset.Clear() call manager.reset().
       -As a consequence, any PolyTree filled with Clipper.Execute() or ClipperLib.Execute()
        cannot be used after calling Clear().
       -This is quite confusing, especially for people used to the old way to use ClipperLib.
        To make the issue more explicit, Clipper and ClipperOffset were modified to own the
        PolyTree after the .Execute(), and client code only gets a pointer to it.
   TAKEAWAY: PolyTree data MUST be processed BEFORE calling clipper.Clear() or ClipperOffset.Clear().*/
class PolyTree: public PolyNode
{ 
protected:
public:
    ~PolyTree(){ Clear(); };
    PolyNode* GetFirst() const;
    void Clear();
    int Total() const;
private:
    //PolyTree& operator =(PolyTree& other);
    PolyNodes AllNodes;
    friend class Clipper; //to access AllNodes
};

bool Orientation(const Path &poly);
double Area(const Path &poly);
int PointInPolygon(const IntPoint &pt, const Path &path);

void SimplifyPolygon(CLIPPER_MMANAGER &manager, const Path &in_poly, Paths &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(CLIPPER_MMANAGER &manager, const Paths &in_polys, Paths &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(CLIPPER_MMANAGER &manager, Paths &polys, PolyFillType fillType = pftEvenOdd);

void CleanPolygon(CLIPPER_MMANAGER &manager, const Path& in_poly, Path& out_poly, double distance = 1.415);
void CleanPolygon(CLIPPER_MMANAGER &manager, Path& poly, double distance = 1.415);
void CleanPolygons(CLIPPER_MMANAGER &manager, const Paths& in_polys, Paths& out_polys, double distance = 1.415);
void CleanPolygons(CLIPPER_MMANAGER &manager, Paths& polys, double distance = 1.415);

void MinkowskiSum(CLIPPER_MMANAGER &manager, const Path& pattern, const Path& path, Paths& solution, bool pathIsClosed);
void MinkowskiSum(CLIPPER_MMANAGER &manager, const Path& pattern, const Paths& paths, Paths& solution, bool pathIsClosed);
void MinkowskiDiff(CLIPPER_MMANAGER &manager, const Path& poly1, const Path& poly2, Paths& solution);

void PolyTreeToPaths(const PolyTree& polytree, Paths& paths);
void ClosedPathsFromPolyTree(const PolyTree& polytree, Paths& paths);
void OpenPathsFromPolyTree(PolyTree& polytree, Paths& paths);

void ReversePath(Path& p);
void ReversePaths(Paths& p);

struct IntRect { cInt left; cInt top; cInt right; cInt bottom; };

//enums that are used internally ...
enum EdgeSide { esLeft = 1, esRight = 2};

//forward declarations (for stuff used internally) ...
struct TEdge;
struct IntersectNode;
struct LocalMinimum {
  cInt          Y;
  TEdge        *LeftBound;
  TEdge        *RightBound;
};
struct OutPt;
struct OutRec;
struct Join;

typedef std::vector < OutRec* > PolyOutList;
typedef std::vector < TEdge* > EdgeList;
typedef std::vector < Join* > JoinList;
typedef std::vector < IntersectNode* > IntersectList;

//------------------------------------------------------------------------------

//ClipperBase is the ancestor to the Clipper class. It should not be
//instantiated directly. This class simply abstracts the conversion of sets of
//polygon coordinates into edge objects that are stored in a LocalMinima list.
class ClipperBase
{
public:
  ClipperBase(CLIPPER_MMANAGER &_manager);
  virtual ~ClipperBase();
  virtual bool AddPath(const Path &pg, PolyType PolyTyp, bool Closed);
  bool AddPaths(const Paths &ppg, PolyType PolyTyp, bool Closed);
  virtual void Clear();
  IntRect GetBounds();
  bool PreserveCollinear() {return m_PreserveCollinear;};
  void PreserveCollinear(bool value) {m_PreserveCollinear = value;};
  CLIPPER_MMANAGER &manager;
protected:
  void DisposeLocalMinimaList();
  TEdge* AddBoundsToLML(TEdge *e, bool IsClosed);
  virtual void Reset();
  TEdge* ProcessBound(TEdge* E, bool IsClockwise);
  void InsertScanbeam(const cInt Y);
  bool PopScanbeam(cInt &Y);
  bool LocalMinimaPending();
  bool PopLocalMinima(cInt Y, const LocalMinimum *&locMin);
  OutRec* CreateOutRec();
  void DisposeAllOutRecs();
  void DisposeOutRec(PolyOutList::size_type index);
  void SwapPositionsInAEL(TEdge *edge1, TEdge *edge2);
  void DeleteFromAEL(TEdge *e);
  void UpdateEdgeIntoAEL(TEdge *&e);

  typedef std::vector<LocalMinimum> MinimaList;
  MinimaList::iterator m_CurrentLM;
  MinimaList           m_MinimaList;

  PolyTree          m_PolyTreeSolution;
  bool              m_UsingPolyTree;
  bool              m_UseFullRange;
  EdgeList          m_edges;
  bool              m_PreserveCollinear;
  bool              m_HasOpenPaths;
  PolyOutList       m_PolyOuts;
  TEdge           *m_ActiveEdges;

  typedef std::priority_queue<cInt, std::vector<cInt> > ScanbeamList;
  ScanbeamList     m_Scanbeam;
};
//------------------------------------------------------------------------------

class Clipper : public ClipperBase
{
public:
  Clipper(CLIPPER_MMANAGER &_manager, int initOptions = 0);
  bool Execute(ClipType clipType,
      Paths &solution,
      PolyFillType fillType = pftEvenOdd);
  bool Execute(ClipType clipType,
      Paths &solution,
      PolyFillType subjFillType,
      PolyFillType clipFillType);
  //WARNING: the returned PolyTree will be emptied when calling Clear()
  bool Execute(ClipType clipType,
      PolyTree *&solution,
      PolyFillType fillType = pftEvenOdd);
  //WARNING: the returned PolyTree will be emptied when calling Clear()
  bool Execute(ClipType clipType,
      PolyTree *&solution,
      PolyFillType subjFillType,
      PolyFillType clipFillType);
  bool ReverseSolution() { return m_ReverseOutput; };
  void ReverseSolution(bool value) {m_ReverseOutput = value;};
  bool StrictlySimple() {return m_StrictSimple;};
  void StrictlySimple(bool value) {m_StrictSimple = value;};
  //set the callback function for z value filling on intersections (otherwise Z is 0)
#ifdef use_xyz
  void ZFillFunction(ZFillCallback zFillFunc);
#endif
protected:
  virtual bool ExecuteInternal();
private:
  JoinList         m_Joins;
  JoinList         m_GhostJoins;
  IntersectList    m_IntersectList;
  ClipType         m_ClipType;
  typedef std::list<cInt> MaximaList;
  MaximaList       m_Maxima;
  TEdge           *m_SortedEdges;
  bool             m_ExecuteLocked;
  PolyFillType     m_ClipFillType;
  PolyFillType     m_SubjFillType;
  bool             m_ReverseOutput;
  bool             m_StrictSimple;
#ifdef use_xyz
  ZFillCallback   m_ZFill; //custom callback 
#endif
  void SetWindingCount(TEdge& edge);
  bool IsEvenOddFillType(const TEdge& edge) const;
  bool IsEvenOddAltFillType(const TEdge& edge) const;
  void InsertLocalMinimaIntoAEL(const cInt botY);
  void InsertEdgeIntoAEL(TEdge *edge, TEdge* startEdge);
  void AddEdgeToSEL(TEdge *edge);
  bool PopEdgeFromSEL(TEdge *&edge);
  void CopyAELToSEL();
  void DeleteFromSEL(TEdge *e);
  void SwapPositionsInSEL(TEdge *edge1, TEdge *edge2);
  bool IsContributing(const TEdge& edge) const;
  bool IsTopHorz(const cInt XPos);
  void DoMaxima(TEdge *e);
  void ProcessHorizontals();
  void ProcessHorizontal(TEdge *horzEdge);
  void AddLocalMaxPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  OutPt* AddLocalMinPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  OutRec* GetOutRec(int idx);
  void AppendPolygon(TEdge *e1, TEdge *e2);
  void IntersectEdges(TEdge *e1, TEdge *e2, IntPoint &pt);
  OutPt* AddOutPt(TEdge *e, const IntPoint &pt);
  OutPt* GetLastOutPt(TEdge *e);
  bool ProcessIntersections(const cInt topY);
  void BuildIntersectList(const cInt topY);
  void ProcessIntersectList();
  void ProcessEdgesAtTopOfScanbeam(const cInt topY);
  void BuildResult(Paths& polys);
  void BuildResult2(PolyTree& polytree);
  void SetHoleState(TEdge *e, OutRec *outrec);
  void DisposeIntersectNodes();
  bool FixupIntersectionOrder();
  void FixupOutPolygon(OutRec &outrec);
  void FixupOutPolyline(OutRec &outrec);
  bool IsHole(TEdge *e);
  bool FindOwnerFromSplitRecs(OutRec &outRec, OutRec *&currOrfl);
  void FixHoleLinkage(OutRec &outrec);
  void AddJoin(OutPt *op1, OutPt *op2, const IntPoint offPt);
  void ClearJoins();
  void ClearGhostJoins();
  void AddGhostJoin(OutPt *op, const IntPoint offPt);
  bool JoinPoints(Join *j, OutRec* outRec1, OutRec* outRec2);
  void JoinCommonEdges();
  void DoSimplePolygons();
  void FixupFirstLefts1(OutRec* OldOutRec, OutRec* NewOutRec);
  void FixupFirstLefts2(OutRec* InnerOutRec, OutRec* OuterOutRec);
  void FixupFirstLefts3(OutRec* OldOutRec, OutRec* NewOutRec);
#ifdef use_xyz
  void SetZ(IntPoint& pt, TEdge& e1, TEdge& e2);
#endif
  friend class PolyTree;
};
//------------------------------------------------------------------------------

class ClipperOffset 
{
public:
  ClipperOffset(CLIPPER_MMANAGER &_manager, double miterLimit = 2.0, double roundPrecision = 0.25);
  ~ClipperOffset();
  void AddPath(const Path& path, JoinType joinType, EndType endType);
  void AddPaths(const Paths& paths, JoinType joinType, EndType endType);
  void Execute(Paths& solution, double delta);
  //WARNING: the returned PolyTree will be emptied when calling Clear()
  void Execute(PolyTree *&solution, double delta);
  void Clear();
  double MiterLimit;
  double ArcTolerance;
  CLIPPER_MMANAGER &manager;
private:
  Paths m_destPolys;
  Path m_srcPoly;
  Path m_destPoly;
  std::vector<DoublePoint> m_normals;
  double m_delta, m_sinA, m_sin, m_cos;
  double m_miterLim, m_StepsPerRad;
  IntPoint m_lowest;
  PolyNode m_polyNodes;
  Clipper clpr;

  void FixOrientations();
  void DoOffset(double delta);
  void OffsetPoint(int j, int& k, JoinType jointype);
  void DoSquare(int j, int k);
  void DoMiter(int j, int k, double r);
  void DoRound(int j, int k);
  friend class PolyTree;
};
//------------------------------------------------------------------------------

class clipperException : public std::exception
{
  public:
    clipperException(const char* description): m_descr(description) {}
    virtual ~clipperException() throw() {}
    virtual const char* what() const throw() {return m_descr.c_str();}
  private:
    std::string m_descr;
};
//------------------------------------------------------------------------------

} //ClipperLib namespace

#endif //clipper_hpp


