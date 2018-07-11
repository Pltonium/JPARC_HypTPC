/*
  PrimaryGeneratorAction.hh
  2017/10  Yang
*/

#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "CLHEP/Vector/LorentzVector.h"
#include <vector>
#include <TLorentzVector.h>

class DetectorConstruction;
class AnalysisManager;
class EvtGen;
class EvtVector4R;
class G4ParticleGun;
class G4ParticleTable;
//class HepLorentzVector;
class EvtStdHep;
class EvtParticle;

typedef CLHEP::Hep3Vector ThreeVector;
typedef CLHEP::HepLorentzVector HepLorentzVector;


class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction( DetectorConstruction *det,
			  AnalysisManager *anaManager=0,
			  EvtGen *evtgen=0);
  ~PrimaryGeneratorAction();
  /*
  PrimaryGeneratorAction( DetectorConstruction *det,
			  Analysis *anaManager=0,
			  EvtGen *evtGen=0);
  ~PrimaryGeneratorAction();
  */
public:
  void GeneratePrimaries( G4Event *anEvent );
  void GenerateTestKaon( G4Event *anEvent, G4ThreeVector D, G4ThreeVector P );
  void GenerateTestPion( G4Event *anEvent, G4ThreeVector D, G4ThreeVector P, int C );
  void GenerateTest72( G4Event *anEvent, EvtGen *evt, G4ThreeVector D, G4ThreeVector P);
  void GenerateTest45( G4Event *anEvent, EvtGen *evt, G4ThreeVector D, G4ThreeVector P, int C, int momMode);

protected:
  DetectorConstruction *det_;
  AnalysisManager *anaMan_;
  EvtGen *evtgen_;
  //BeamParam *BP_;
  G4ParticleTable* particleTable;

  G4ParticleGun *chooseGun( int Pid );
  G4ParticleGun *chooseGun( const G4String & name );
  G4ParticleGun *particleGun;

  void makeGun(G4Event *anEvent, int partnum, EvtVector4R x, EvtVector4R y);
  void DeleteGuns();
  void GenerateDecay(G4Event* anEvent, EvtGen *evtGenerator, EvtParticle* particle, G4ThreeVector D);
  void GenerateDecay_angle(G4Event* anEvent, EvtGen *evtGenerator, EvtParticle* particle, G4ThreeVector P, G4ThreeVector D);
  double GetCos(TLorentzVector lv1, TLorentzVector lv2);

  // Beam //
  double bpx_; //beam momentum
  double bpy_;
  double bpz_;
  double bvx_; //beam vertex
  double bvy_;
  double bvz_;
  double sigmabvx_; //beam vertex sigma
  double sigmabvy_;
  double sigmabvz_;

private:
  //AnalysisManager* anaMgr;

};

#endif
