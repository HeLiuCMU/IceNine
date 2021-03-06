/*
*
*
*  File:  MicIO.h
*  E-mail:  sfli@cmu.edu
*  Author:  Frankie Li
*  Input output operations for mic files.
*  
*
*  !!! IMPORTANT !!!
*  All MIC files are written in degrees.  MicIO automatically converts
*  all angle information from degrees to radian.  When a MIC file is written
*  using MicIO, radians are automatically converted back to degress.  Internal
*  representation of angular information is always in RADIANS!!!!
*/

#ifndef _MIC_IO_H_
#define _MIC_IO_H_


#include "Debug.h"
#include "Parser.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Voxel.h"
#include <assert.h>
#include "Types.h"
#include "Serializer.h"
#include <boost/shared_ptr.hpp>
#include <cmath>

using std::cerr;
using std::cout;
using std::endl;
using std::skipws;
using std::fstream;
using std::ifstream;
using std::vector;
using std::string;

enum GridType{
  eTriangular,
  eSquare,
  eNumGridTypes
};



//-----------------------
//  Forward Declarations
//-----------------------
template< class VoxelType >
class MicFile;


typedef MicFile< SVoxel > CMic;


//---------------------------------------------------
//  Abstract class
//---------------------------------------------------
class MicIOBase
{
public:
  typedef boost::shared_ptr< MicIOBase > MicIOBasePtr;
  
  virtual      ~MicIOBase(){}
  virtual bool Read( const string &filename )              = 0;
  virtual bool Write( const string &filename ) const       = 0;
  virtual bool Save   ( CSerializer   & oSerialBuf ) const = 0;
  virtual bool Restore( CDeserializer & oSerialBuf )       = 0;
  virtual bool InitializeToResolution( Float & fMinRes )   = 0;
  virtual bool SetMinResolution( Float fMinRes )           = 0;

  //---------------------------------------------------
  //  InitializeSampleLimits
  //
  //  - HACK:  This is a hack so that there's a generalized
  //           way to initialize the sample limits regardless
  //           of the exact file format.
  //---------------------------------------------------
  virtual void InitializeSampleLimits( const MicIOBase * pMicPtr ) = 0;
};



//-----------------------
//
//  MicFile
//
//-----------------------

template< class VoxelType >
class MicFileBase : public MicIOBase
{
protected:
  vector<VoxelType> oVoxelList;
  Float             fInitialSideLength;
  Int               nNumCols;

  char *pBuffer;
  LInt nBufferSize;
  
  void ClearBuffer();
  bool ReadFileToBuf(const string &filename);

  Int  GetLeftMostVertex( const VoxelType & v ) const;
  
public:
  MicFileBase():	oVoxelList(), fInitialSideLength(0), nNumCols(10),
                pBuffer(NULL), nBufferSize(0){};

  virtual ~MicFileBase();

  //-----------------------------------
  //  SetNumCol
  //  Action:  Sets the number of columns in a MIC file.
  //  Note that by default, MIC files are assumed to have 10 columns.
  //-----------------------------------
  void SetNumCol( Int newNumCols );

  //-----------------------------------
  //  SetVoxelList
  //-----------------------------------
  void SetVoxelList( const vector<VoxelType> & oVoxels );

  //-----------------------------------
  //  Add Voxel
  //-----------------------------------
  void AddVoxel( const VoxelType & oVoxel );
  
  //-----------------------------------
  //  SetInitialSideLength
  //-----------------------------------
  void SetInitialSideLength( Float fSideLength );

  //-----------------------------------
  //  SetID
  //  Set ID of voxel, starting from nStartID,
  //  and return the next ID to be used.
  //-----------------------------------
  Int SetID( Int nStartID );
  
  //-----------------------------------
  //  Read
  //  Action: Read the MIC files located at filename.
  //  Return true if succes, false otehrwise.
  //-----------------------------------
  virtual bool Read( const string &filename ) = 0;

  //-----------------------------------
  //  Write
  //  Action:  Write current object to the filename specified
  //  return false if failed.
  //-----------------------------------
  virtual bool Write( const string &filename ) const = 0;
  
  //-----------------------------------
  //  Accessors
  //-----------------------------------
  
  //-----------------------------------
  //  GetInitialSideLength
  //-----------------------------------
  Float GetInitialSideLength() const;

  //-----------------------------------
  //  GetNumVoxels
  //-----------------------------------
  Float GetNumVoxels() const;

  //-----------------------------------
  //  GetMinSideLength
  //-----------------------------------
  Float GetMinSideLength() const;
  
  //-----------------------------------
  // Clear
  //-----------------------------------
  void Clear();

  //-----------------------------------
  //  GetVoxels
  //  Purpose:  Return vector of VoxelType from current structure
  //  WARNING!
  //  Use return of reference to vector with EXTREME CAUTION
  //-----------------------------------
  const vector<VoxelType> & GetVoxels() const { return oVoxelList; }
  vector<VoxelType> &       GetVoxels()       { return oVoxelList; }
  
  //-----------------------------------
  //  Iterators
  //-----------------------------------
  typedef typename vector<VoxelType>::iterator       VoxelType_iterator; 
  typedef typename vector<VoxelType>::const_iterator VoxelType_const_iterator;
  VoxelType_iterator       VoxelListBegin()       { return oVoxelList.begin(); }
  VoxelType_const_iterator VoxelListBegin() const { return oVoxelList.begin(); }
  VoxelType_iterator       VoxelListEnd  ()       { return oVoxelList.end();   }
  VoxelType_const_iterator VoxelListEnd  () const { return oVoxelList.end();   }
  
  virtual bool InitializeToResolution ( Float & fMinRes )
  {
    RUNTIME_ASSERT(0, "Initialize to Resolution does not work for MicFileBase");
  }
  virtual bool SetMinResolution       ( Float fMinRes )
  {
    RUNTIME_ASSERT(0, "Set Min Set min Resolution does not work for MicFileBase");
  }
  virtual void InitializeSampleLimits( const MicIOBase * pMicPtr )
  {
    RUNTIME_ASSERT(0, "Set Min Initialze Sample Limits Resolution does not work for MicFileBase");
  }
};


//-----------------------------------
//  MicFile<VoxelType>
//   ---  Abstract class
//
//-----------------------------------
template< class VoxelType >
class MicFile : public MicFileBase< VoxelType >
{
public:
  typedef MicFileBase< VoxelType >                   Base;
  typedef typename vector<VoxelType>::iterator       VoxelType_iterator; 
  typedef typename vector<VoxelType>::const_iterator VoxelType_const_iterator;

  MicFile():  Base() {}
  
  virtual bool Read( const string &filename ) = 0;
 
  virtual bool Write( const string &filename ) const = 0;
  virtual void InitializeSampleLimits( const MicIOBase * pMicPtr )  = 0;  

};

//----------------------------------------------------------------------
//
//
//  MicFile<SVoxel>
//
//
//----------------------------------------------------------------------
template<>
class MicFile< SVoxel > : public MicFileBase< SVoxel >
{
public:
  typedef SVoxel VoxelType;
  typedef vector<VoxelType>::iterator       VoxelType_iterator; 
  typedef vector<VoxelType>::const_iterator VoxelType_const_iterator;
  typedef MicFile<SVoxel>                   Self;
  //-----------------------------------
  //  Read
  //  Action: Read the MIC files located at filename.
  //  Return true if succes, false otehrwise.
  //-----------------------------------
  virtual bool Read( const string &filename )
  {
    vector< vector<string> > vsTokens;
    if( !ReadFileToBuf(filename))
      return false;
    
    GeneralLib::Tokenize( vsTokens, string( pBuffer, nBufferSize ), " \t\n");

    // we expect side length for the first line
    if(vsTokens[0].size() > 1){
      cerr << "[MicFileBase::ReadMicFileBase] Incorrect Mic File Format " << filename << endl;
      exit(0);
    }
    oVoxelList.clear();
    fInitialSideLength = atof(vsTokens[0][0].c_str());
    for( Size_Type i = 1; i < vsTokens.size(); i ++)
    {
      if(vsTokens[i].size() < 9 )
      {
        cerr << "[MicFileBase::ReadMicFile] Error: Incorrect number of columns, line: "
             << i  << filename << endl;
      }
      else
      {
        DEBUG_ALERT( vsTokens[i].size() >= 10, "MicFileBase::Read:  Warning!  Old format MIC file with only 9 columns\n" );
        VoxelType v;

        v.nGeneration = atoi( vsTokens[i][4].c_str() );
        v.fSideLength = fInitialSideLength / pow( 2,  v.nGeneration ) ;
        v.nPhase = atoi( vsTokens[i][5].c_str() );

        Float fX, fY, fZ;

        fX = atof( vsTokens[i][0].c_str() );
        fY = atof( vsTokens[i][1].c_str() );
        fZ = atof( vsTokens[i][2].c_str() );

        if ( atoi( vsTokens[i][3].c_str() ) == UP  ) 
        {
          // Winding order, counter clockwise
          v.pVertex[0].Set( fX,                 fY, fZ );
          v.pVertex[1].Set( fX + v.fSideLength, fY, fZ );
          v.pVertex[2].Set( fX + v.fSideLength / (Float) 2.0 , fY + v.fSideLength / (Float) 2.0 * sqrt( (Float) 3.0 ) , fZ);
          v.bVoxelPointsUp = true;
        }
        else
        {
          // Winding order, counter clockwise
          v.pVertex[0].Set( fX,                 fY, fZ );
          v.pVertex[1].Set( fX + v.fSideLength / (Float) 2.0 , fY - v.fSideLength / (Float) 2.0 * sqrt( (Float) 3.0 ) , fZ);
          v.pVertex[2].Set( fX + v.fSideLength, fY, fZ );
          v.bVoxelPointsUp = false;
        }

        SVector3 oOrientation;
        oOrientation.Set(  DEGREE_TO_RADIAN( atof(vsTokens[i][6].c_str()) ),
                           DEGREE_TO_RADIAN( atof(vsTokens[i][7].c_str()) ),
                           DEGREE_TO_RADIAN( atof(vsTokens[i][8].c_str()) ) );
      
        // note that orientation matrix assigned here - the choice of active vs. passive
        // transform is made by the file format

        v.oOrientMatrix.BuildActiveEulerMatrix( oOrientation.m_fX,
                                                oOrientation.m_fY,
                                                oOrientation.m_fZ );
        if( vsTokens[i].size() > 9 )
          v.fConfidence = atof( vsTokens[i][9].c_str() );
        else
          v.fConfidence = 0;
      
        if( vsTokens[i].size() == 19 )   //  Defining our strain tensor to be ( I)
        {
          v.fCost                = atof( vsTokens[i][10].c_str() );
          v.fPixelOverlapRatio   = atof( vsTokens[i][11].c_str() );
          v.oDeformation.m[0][0] = atof( vsTokens[i][13].c_str() );
          v.oDeformation.m[1][1] = atof( vsTokens[i][14].c_str() );
          v.oDeformation.m[2][2] = atof( vsTokens[i][15].c_str() );
        
          v.oDeformation.m[0][1] = v.oDeformation.m[1][0] = atof( vsTokens[i][16].c_str() );
          v.oDeformation.m[1][2] = v.oDeformation.m[2][1] = atof( vsTokens[i][17].c_str() );
          v.oDeformation.m[0][2] = v.oDeformation.m[2][0] = atof( vsTokens[i][18].c_str() );
        }
        else
        {
          v.oDeformation.SetIdentity();
        }
      
        oVoxelList.push_back(v);
      }	
    }
    
    ClearBuffer();  // clear buffer after use
    return true;
  }
  
  //-----------------------------------
  //  Write
  //  Action:  Write current object to the filename specified
  //  return false if failed.
  //-----------------------------------
  virtual bool Write( const string &filename ) const
  {
    std::ofstream osOutFile;
    osOutFile.open( filename.c_str() );

    if( !osOutFile.good() )
      return false;

    osOutFile << fInitialSideLength << std::endl;
  
    // columns 1 -3: xyz location of left vertex of a triangle
    // column 4: 1/2 for triangle type -- 1 for upward pointing or 2 for downward
    // pointing
    // column 5: generation number -- triangle side length = a0/(2**generation),
    // generation = 0,1,2,...
    // column 6: 0/1 for phase -- 0 mean it wasn't (or hasn't yet) fitted to an
    // orientation, 1 means it was
    // columns 7 -9: Eulers in degrees
    // column 10: confidence parameter: fraction of simulated Bragg peaks that hit
    // experimental peaks
  
    for( Size_Type i = 0; i < oVoxelList.size(); i ++ )
    {
      Int nDir;
      Int nLeftMostVertex;

      //  Need to find the left most voxel
      //  to follow MIC files specification.
      nLeftMostVertex = GetLeftMostVertex( oVoxelList[i] );

      if ( oVoxelList[i].bVoxelPointsUp ){
        nDir = UP;
      }else{
        nDir = DOWN;
      }
    
      osOutFile << oVoxelList[i].pVertex[nLeftMostVertex].m_fX << " "
                << oVoxelList[i].pVertex[nLeftMostVertex].m_fY  << " "
                << oVoxelList[i].pVertex[nLeftMostVertex].m_fZ << " "

                << nDir << " "
                << oVoxelList[i].nGeneration << " "
                << oVoxelList[i].nPhase << " "
                << RadianToDegree( oVoxelList[i].oOrientMatrix.GetEulerAngles() ) << " "
                << oVoxelList[i].fConfidence
      
        //---------------------------------------------------------
        // newly added - to be put into property map
        //---------------------------------------------------------
                << " " << oVoxelList[i].fCost
                << " " << oVoxelList[i].fPixelOverlapRatio
                << " " << oVoxelList[i].fFittingTime
      
                << " " << oVoxelList[i].oDeformation.m[0][0]
                << " " << oVoxelList[i].oDeformation.m[1][1]
                << " " << oVoxelList[i].oDeformation.m[2][2]
                << " " << oVoxelList[i].oDeformation.m[1][0]
                << " " << oVoxelList[i].oDeformation.m[1][2]
                << " " << oVoxelList[i].oDeformation.m[2][0]
      
                << std::endl;;
    }

    osOutFile.close();

    return true;
  }
    
  //----------------------------------------------------------
  // Save
  //----------------------------------------------------------
  virtual bool  Save   ( CSerializer   & oSerialBuf ) const
  {
    bool bSuccess = oSerialBuf.InsertCompactObj   ( GetInitialSideLength() );
    bSuccess = bSuccess && oSerialBuf.InsertCompactVector( oVoxelList );
    return bSuccess;
  }

  //----------------------------------------------------------
  //  Restore
  //----------------------------------------------------------
  virtual bool  Restore( CDeserializer & oSerialBuf ) 
  {
    Float fSideWidth;
    bool bSuccess =        oSerialBuf.GetCompactObj   ( &fSideWidth );
    bSuccess = bSuccess && oSerialBuf.GetCompactVector( oVoxelList  );
    SetInitialSideLength( fSideWidth );
    return bSuccess;
  }

  virtual bool  InitializeToResolution( Float & fMinRes )
  {
    RUNTIME_ASSERT(0, "InitializeWithResolution is NOT IMPLEMENTED for Triangular mic");
    return false;
  }

  virtual bool  SetMinResolution( Float fMinRes )
  {
    RUNTIME_ASSERT(0, "SetMinResolution not NOT IMPLEMENTED for Triangular mic\n");
    return false;
  }

  //----------------------------------------------------------
  //  InitializeSampleLimits
  //  MicFile<SVoxel> doesn't actually have SampleOrigin, and VoxelSideLength,
  //  so those variables are just dummies.
  //
  //  SampleSideLength <==>  InitialSideLength in SVoxel
  //
  //----------------------------------------------------------
  void InitializeSampleLimits( const MicIOBase * pMicPtr )
  {
    SetInitialSideLength( dynamic_cast< const Self*>(pMicPtr)->GetInitialSideLength() );
  }
  
};



//----------------------------------------------------------------------
//
//
//  MicFile<SquareVoxel>
//
//
//----------------------------------------------------------------------
template<>
class MicFile< SquareVoxel > : public MicFileBase< SquareVoxel >
{
  
public:
  
  typedef SquareVoxel                       VoxelType;
  typedef vector<VoxelType>::iterator       VoxelType_iterator; 
  typedef vector<VoxelType>::const_iterator VoxelType_const_iterator;
  typedef MicFile<SquareVoxel>              Self;
private:
  
  void SubdivideVoxel( vector<VoxelType> & VoxelList, const VoxelType v,
                       Int NumSubdivisions )
  {
    Float fNewSideLength = v.fSideLength / static_cast<Float>( NumSubdivisions );
    SVector3 pMin = v.pVertex[0] + SVector3( fNewSideLength * 0.5, fNewSideLength * 0.5, 0 );
    for( int i = 0; i < NumSubdivisions; i ++)
      for( int j = 0; j < NumSubdivisions; j ++)
      {
        SVector3  NewCenter = pMin + SVector3( i * fNewSideLength, j * fNewSideLength, 0 );
        VoxelType NewVoxel = v;
        NewVoxel.SetCenter( NewCenter, fNewSideLength );
        VoxelList.push_back( NewVoxel );
      }
  }
  
public:
  
  //--------------------------------------------------------------------
  //  This section to be changed to RectMicGrid< SquareVoxel > soon
  SVector3                                  Origin;   // a square grid
  Float                                     VoxelSideLength;
  Float                                     SampleSideLength;
  bool                                      Initialized;
  //--------------------------------------------------------------------

  MicFile( const SVector3 & Origin_, Float SampleSideLength_, Float VoxelSideLength_ )
    : Origin( Origin_ ),
      VoxelSideLength( VoxelSideLength_),
      SampleSideLength( SampleSideLength_ ),
      Initialized( true ) {}

  MicFile() : Initialized( false ){}
  

  
  //-----------------------------------
  //  Read
  //  Action: Read the MIC files located at filename.
  //  Return true if succes, false otehrwise.
  //-----------------------------------
  virtual bool Read( const string &filename )
  {
    vector< vector<string> > vsTokens;
    if( !ReadFileToBuf(filename))
      return false;
    
    GeneralLib::Tokenize( vsTokens, string( pBuffer, nBufferSize ), " \t\n");

    // we expect side length for the first line
    if( (vsTokens.size() < 2 )  || (vsTokens[0].size()  != 2 ) || ( vsTokens[1].size() != 3 ) )
    {
      cerr << "[MicFile<SquareVoxel>] Incorrect Mic File Format: " << filename << endl << "Expecting square grid mic format " << endl;
      exit(0);
    }

    SampleSideLength = atof(vsTokens[0][0].c_str());        // *Sample* side length
    VoxelSideLength  = atof(vsTokens[0][1].c_str());        // *Voxel*  side length

    Origin.m_fX = atof( vsTokens[1][0].c_str() );
    Origin.m_fY = atof( vsTokens[1][1].c_str() );
    Origin.m_fZ = atof( vsTokens[1][2].c_str() );

    std::cout << "Reading Square Grid Mic File: " << SampleSideLength << " " << VoxelSideLength << std::endl
              << Origin << std::endl;

    oVoxelList.clear();
    for( Size_Type i = 2; i < vsTokens.size(); i ++)  // start from 2
    {
      VoxelType v;
      
      v.fSideLength = VoxelSideLength;
      Float fX, fY, fZ;
      fX = atof( vsTokens[i][0].c_str() );
      fY = atof( vsTokens[i][1].c_str() );
      fZ = atof( vsTokens[i][2].c_str() );
      v.nPhase = atoi( vsTokens[i][3].c_str() );
 
      v.SetCenter( SVector3( fX, fY, fZ ) );
      SVector3 oOrientation;
      oOrientation.Set( DEGREE_TO_RADIAN( atof(vsTokens[i][4].c_str()) ),
                        DEGREE_TO_RADIAN( atof(vsTokens[i][5].c_str()) ),
                        DEGREE_TO_RADIAN( atof(vsTokens[i][6].c_str()) ) );
      
      // note that orientation matrix assigned here - the choice of active vs. passive
      // transform is made by the file format
        
      v.oOrientMatrix.BuildActiveEulerMatrix( oOrientation.m_fX,
                                              oOrientation.m_fY,
                                              oOrientation.m_fZ );
      v.fConfidence = atof( vsTokens[i][7].c_str() );
      
      if( vsTokens[i].size() >= 10 )   // if there's extra information
      {
        v.fCost                = atof( vsTokens[i][8].c_str() );
        v.fPixelOverlapRatio   = atof( vsTokens[i][9].c_str() );
      }
      oVoxelList.push_back(v);
    }

    Initialized = true;
    ClearBuffer();  // clear buffer after use
    return true;
  }
  
  //-----------------------------------
  //  Write
  //  Action:  Write current object to the filename specified
  //  return false if failed.
  //-----------------------------------
  virtual bool Write( const string &filename ) const
  {
    std::ofstream osOutFile;
    osOutFile.open( filename.c_str() );

    if( !osOutFile.good() )
      return false;
    
    osOutFile << SampleSideLength << " " << VoxelSideLength << std::endl;
    osOutFile << Origin << std::endl;
    // columns 0-2: xyz location of left vertex of a triangle
    // column  3:     Phase
    // columns 4-6: Eulers in degrees
    // column  7: confidence parameter: fraction of simulated Bragg peaks that hit
    // experimental peaks
  
    for( Size_Type i = 0; i < oVoxelList.size(); i ++ )
    {
      osOutFile << oVoxelList[i].GetCenter() << " "
                << oVoxelList[i].nPhase      << " "
                << RadianToDegree( oVoxelList[i].oOrientMatrix.GetEulerAngles() ) << " "
                << oVoxelList[i].fConfidence
        
                << " " << oVoxelList[i].fCost
                << " " << oVoxelList[i].fPixelOverlapRatio
                << " " << oVoxelList[i].fFittingTime
                << std::endl;;
    }

    osOutFile.close();

    return true;
  }
  
  //----------------------------------------------------------
  // Save
  //----------------------------------------------------------
  virtual bool  Save   ( CSerializer   & oSerialBuf ) const
  {
    bool bSuccess = oSerialBuf.InsertCompactObj   ( GetInitialSideLength() );
    bSuccess = bSuccess && oSerialBuf.InsertCompactObj   ( Origin );
    bSuccess = bSuccess && oSerialBuf.InsertCompactObj   ( VoxelSideLength );
    bSuccess = bSuccess && oSerialBuf.InsertCompactObj   ( SampleSideLength );
    bSuccess = bSuccess && oSerialBuf.InsertCompactObj   ( Initialized );
    
    bSuccess = bSuccess && oSerialBuf.InsertCompactVector( oVoxelList );
    return bSuccess;
  }

  //----------------------------------------------------------
  //  Restore
  //----------------------------------------------------------
  virtual bool  Restore( CDeserializer & oSerialBuf )
  {
    Float fSideWidth;
    bool bSuccess =        oSerialBuf.GetCompactObj   ( &fSideWidth );
    bSuccess = bSuccess && oSerialBuf.GetCompactObj   ( &Origin );
    bSuccess = bSuccess && oSerialBuf.GetCompactObj   ( &VoxelSideLength );
    bSuccess = bSuccess && oSerialBuf.GetCompactObj   ( &SampleSideLength );
    bSuccess = bSuccess && oSerialBuf.GetCompactObj   ( &Initialized );
    
    bSuccess = bSuccess && oSerialBuf.GetCompactVector( oVoxelList  );
    SetInitialSideLength( fSideWidth );
    return bSuccess;
  }
  
  //----------------------------------------------------------
  //  InitializeToResolution
  //----------------------------------------------------------
  bool InitializeToResolution( Float fMinRes )
  {
    
    if( !Initialized )
      return false;
    
    if( fMinRes < VoxelSideLength )
      VoxelSideLength = fMinRes;
    
    int VoxelPerSide = std::ceil( SampleSideLength / VoxelSideLength ); 
    oVoxelList.clear();
    SVector3 OriginShift( 0.5 * VoxelSideLength, 0.5 * VoxelSideLength, 0 );
    
    for( Int i = 0; i < VoxelPerSide; i ++ )
    {
      for( Int j = 0; j < VoxelPerSide; j ++ )
      {
        VoxelType Voxel;
        SVector3 vLoc = Origin + OriginShift
          + SVector3( i *  VoxelSideLength, j *  VoxelSideLength, 0);
        Voxel.SetCenter( vLoc, VoxelSideLength );
        oVoxelList.push_back( Voxel );
      }
    }
    return false;
  }

  //----------------------------------------------------------
  //  SetMinResolution
  //   All voxels in the Mic file will be set to at least the minimum
  //   resolution.
  //
  //  -- should switch to in-place replacement
  //----------------------------------------------------------
  bool SetMinResolution( Float fMinRes )
  {
    RUNTIME_ASSERT( Initialized, " SetMinResolution called without initialization\n" );

    int NumSideDivisions = 1;
    if( VoxelSideLength > fMinRes )
    {
      NumSideDivisions = std::ceil( VoxelSideLength / fMinRes );
      VoxelSideLength /= static_cast<Float>( NumSideDivisions );
    }
    vector<VoxelType> NewVoxelList;
    for( Size_Type i = 0; i < oVoxelList.size(); i ++ )
    {
      if( oVoxelList[i].fSideLength > fMinRes )
        SubdivideVoxel( NewVoxelList, oVoxelList[i], NumSideDivisions );
      else
        NewVoxelList.push_back ( oVoxelList[i] );
    }
    oVoxelList = NewVoxelList;

    return true;
  }

  void Initialize( const SVector3 & Origin_, Float SampleSideLength_, Float VoxelSideLength_ )
  {
    Origin           = Origin_;
    SampleSideLength = SampleSideLength_;
    VoxelSideLength  = VoxelSideLength_;
    Initialized      = true;
  }

  //----------------------------------------------------------
  //  InitializeSampleLimits
  //  InitializeSampleLimits acts as an abstraction to Initialize
  //----------------------------------------------------------
  void InitializeSampleLimits ( const MicIOBase * pMicPtr )
  {
    const Self* Ptr = dynamic_cast< const Self*>(pMicPtr);
    
    std::cout << " [] ------- [] " << Ptr->Origin
              << " || " << Ptr->SampleSideLength
              << " || " << Ptr->VoxelSideLength <<  std::endl;
    
    Initialize( Ptr->Origin, Ptr->SampleSideLength, Ptr->VoxelSideLength );
  }
  
};


//----------------------------------------------------------
//  MicIOFactory
//----------------------------------------------------------
class MicIOFactory
{
public:
  typedef MicIOBase::MicIOBasePtr            MicIOPtr;
  typedef MicFile<SVoxel>                    TriangularMic;
  typedef MicFile<SquareVoxel>               SquareMic;
  //----------------------------------------
  //  Factory that generates MicFile
  //----------------------------------------
  static MicIOPtr Create( int nGridType )
  {
    switch( nGridType )
    {
      case eTriangular:
        {
          std::cout << "Created Triangular mic " << std::endl;
          MicIOPtr p = MicIOPtr( new TriangularMic() ) ;   // named ptr from "Best Practice" of boost::shared_ptr
          return p;
        }
      case eSquare:
        {
          std::cout << "Created Square mic " << std::endl;
          MicIOPtr p = MicIOPtr( new SquareMic() ) ;
          return p;
        }
      default:
        {
          std::cout << "failed " << std::endl;
          RUNTIME_ASSERT( 0, "MicFileFactory: Unexpected GridType.  Cannot create object \n");
        }
    }
    std::cout << "failed ---  " << std::endl;
    return MicIOPtr();
  }
  
};


#include "MicIO.tmpl.cpp"

#endif
