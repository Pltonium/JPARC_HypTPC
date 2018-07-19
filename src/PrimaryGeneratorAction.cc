/*
  PrimaryGeneratorAction.cc

  2017/10  Yang
*/

#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "AnalysisManager.hh"
#include "ConfMan.hh"

#include "globals.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4LorentzVector.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"

#include "Randomize.hh"

#include "EvtGen/EvtGen.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtParticleFactory.hh"
#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtRandom.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtHepMCEvent.hh"
#include "EvtGenBase/EvtStdlibRandomEngine.hh"
#include "EvtGenBase/EvtAbsRadCorr.hh"
#include "EvtGenBase/EvtDecayBase.hh"
#include "EvtGenExternal/EvtExternalGenList.hh"

#include "EvtGenBase/EvtVector4R.hh"
#include "EvtGenBase/EvtVectorParticle.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtScalarParticle.hh"
#include "EvtGenBase/EvtDecayTable.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtStdHep.hh"
#include "EvtGenBase/EvtSecondary.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtRandom.hh"
#include "EvtGenBase/EvtRandomEngine.hh"
#include "EvtGenBase/EvtParticleFactory.hh"
#include "CLHEP/Vector/LorentzVector.h"
#include "EvtGenBase/EvtStatus.hh"
#include "EvtGenBase/EvtAbsRadCorr.hh"
#include "EvtGenBase/EvtRadCorr.hh"

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <zlib.h>

#include <TRandom.h>
#include <TMath.h>
#include <TF1.h>
#include <TVector3.h>
#include <TLorentzVector.h>

//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/copy.hpp>
//#include <boost/iostreams/filter/gzip.hpp>

static int prmclass=0;
static int prmclass2=0;

PrimaryGeneratorAction::PrimaryGeneratorAction( DetectorConstruction *det,
						AnalysisManager *analysisManager,
						EvtGen *evtgen)
  : G4VUserPrimaryGeneratorAction(), det_(det),
    anaMan_(analysisManager),
    evtgen_(evtgen)
{

  G4int n_particle = 1;
  particleGun = new G4ParticleGun(n_particle);
  particleTable = G4ParticleTable::GetParticleTable();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  //if(BP_) delete BP_;
  DeleteGuns();
}

void PrimaryGeneratorAction::GeneratePrimaries( G4Event *anEvent )
{
  //std::cout<<"Beam generated "<< std::endl;
  G4int event_num = anEvent->GetEventID()+1;
  //G4cout<<"Events: "<< event_num <<G4endl;
  if(event_num% 1000 == 0)
    {
      G4cout<<"Event# "<<event_num<<G4endl;
    }

  ConfMan *confMan = ConfMan::GetConfManager();
  int reactionMode = confMan->ReactionMode();
  int momentum_selection = confMan->BeamMomentumMode();
  int C = confMan->PionCharge();

  //vertex point and beam momentum
  bvx_ = confMan->GetBeamVX();
  bvy_ = confMan->GetBeamVY();
  bvz_ = confMan->GetBeamVZ();
  G4ThreeVector D(bvx_, bvy_, bvz_);
  bpx_ = confMan->GetBeamPX();
  bpy_ = confMan->GetBeamPY();
  bpz_ = confMan->GetBeamPZ();
  G4ThreeVector P(bpx_, bpy_, bpz_);
  anaMan_->SetBeam(1, D, P);

  switch(reactionMode)
    {
    case 1: GenerateTestKaon(anEvent, D, P); break;
    case 2: GenerateTestPion(anEvent, D, P, C); break;
    case 3: GenerateTest72(anEvent, evtgen_, D, P); break;
    case 4: GenerateTest45(anEvent, evtgen_, D, P, C, momentum_selection ); break;
    }
}

void PrimaryGeneratorAction::DeleteGuns()
{
}

G4ParticleGun * PrimaryGeneratorAction::chooseGun( G4int Pid )
{
  //G4String name=PIDParticleName(Pid);
  //return chooseGun( name );
}

G4ParticleGun * PrimaryGeneratorAction::chooseGun( const G4String & name  )
{
}

void PrimaryGeneratorAction::GenerateTestKaon(G4Event* anEvent, G4ThreeVector D, G4ThreeVector P)
{
  double mass_km = 0.493677;
  //particleGun -> SetParticleDefinition (particleTable -> FindParticle("kaonT1"));
  particleGun -> SetParticleDefinition (particleTable -> FindParticle("kaon-"));
  //std::cout<<"Beam energy GenerateTest 2" << std::endl;
  //G4ThreeVector dir = G4RandomDirection();
  G4ThreeVector beamx ( D.x(), D.y(), D.z());
  G4ThreeVector beamp ( P.x(), P.y(), P.z());
  G4ThreeVector beampu =  beamp/beamp.mag();
  G4double energy = (sqrt(mass_km*mass_km+beamp.mag2()) - mass_km )*GeV;
  //std::cout<<"Beam energy: "<< energy /GeV << std::endl;

  particleGun->SetParticleMomentumDirection ( beampu );
  //particleGun->SetParticleMomentum ( beamp );
  particleGun->SetParticleTime ( 0.0 );
  particleGun->SetParticlePosition( beamx );
  particleGun->SetParticleEnergy( energy );
  particleGun->GeneratePrimaryVertex( anEvent);
}

void PrimaryGeneratorAction::GenerateTestPion(G4Event* anEvent, G4ThreeVector D, G4ThreeVector P, int C)
{
  ConfMan *confMan = ConfMan::GetConfManager();

  double mass_pi = 0.13957061;
  if(C==-1) particleGun -> SetParticleDefinition (particleTable -> FindParticle("pi-"));
  else if(C==1) particleGun -> SetParticleDefinition (particleTable -> FindParticle("pi+"));
  else{
    G4cout<<"### Put a pion charge information ###"<<G4endl;
    return;
  }

  double sigmabvx, sigmabvy;
  sigmabvx = confMan->GetBeamVXSig();
  sigmabvy = confMan->GetBeamVYSig();

  int flag=1;
  double TV_bvx, TV_bvy, TV_bvz;
  while(flag){
    TRandom *eventgen = new TRandom(prmclass);
    TV_bvx = eventgen->Gaus(D.x(),sigmabvx);
    TV_bvy = eventgen->Gaus(D.y(),sigmabvy);
    if(TMath::Abs(TV_bvy-D.y())<50.0 && TMath::Abs(TV_bvx-D.x())<27.0)  flag=0;
    prmclass++;
  }
  TV_bvz = D.z();

  //G4ThreeVector dir = G4RandomDirection();
  G4ThreeVector beamx ( TV_bvx, TV_bvy, TV_bvz);
  G4ThreeVector beamp ( P.x(), P.y(), P.z());
  G4ThreeVector beampu =  beamp/beamp.mag();
  G4double energy = (sqrt(mass_pi*mass_pi+beamp.mag2()) - mass_pi )*GeV;
  //std::cout<<"Beam energy: "<< energy /GeV << std::endl;

  particleGun->SetParticleMomentumDirection ( beampu );
  particleGun->SetParticleTime ( 0.0 );
  particleGun->SetParticlePosition( beamx );
  particleGun->SetParticleEnergy( energy );
  particleGun->GeneratePrimaryVertex( anEvent);

}

void PrimaryGeneratorAction::GenerateTest72(G4Event* anEvent, EvtGen *evtGenerator, G4ThreeVector D, G4ThreeVector P)
{
  //G4cout<<"Start Generate Test72"<<G4endl;


  /// mother particle momentum //
  double mass_km = 0.493677;
  double mass_proton = 0.938272081;
  G4LorentzVector lv_beam;
  G4LorentzVector lv_target;
  G4LorentzVector lv_particle;
  double pbeam = P.mag();

  lv_beam.setX(P.x());
  lv_beam.setY(P.y());
  lv_beam.setZ(P.z());
  lv_beam.setE(sqrt(mass_km*mass_km + pbeam*pbeam));

  lv_target.setX(0.0);
  lv_target.setY(0.0);
  lv_target.setZ(0.0);
  lv_target.setE(sqrt(mass_proton*mass_proton));

  lv_particle = lv_beam + lv_target;

  // make mother particle //
  EvtParticle* lam1663(0);
  static EvtId LAM1663 = EvtPDL::getId(std::string("Lambda(1663)0"));
  G4LorentzVector LvLam1663;
  G4ThreeVector TVp (lv_particle.x(), lv_particle.y(), lv_particle.z());
  G4ThreeVector TVx (D.x(), D.y(), D.z());
  //double mass_lam1663 = EvtPDL::getMass(LAM1663);
  double mass_lam1663 = sqrt((lv_beam.e()+lv_target.e())*(lv_beam.e()+lv_target.e()) - pbeam*pbeam);

  // check total energy //
  if(mass_lam1663 < 1.115683 + 0.547862 )
    {
      G4cout<<"### Beam momentum is not enough to generate Lam1663 ###"<<G4endl;
      return;
    }
  /*
  G4cout<<"########################### Test  ##############################"<<G4endl;
  G4cout<<"Momentum of K-: "<<pbeam << " GeV/c" <<G4endl;
  G4cout<<"Invariant mass of K + p: "<<lv_particle.m() << " GeV/c2" <<G4endl;
  G4cout<<"Momentum of Lam1663: "<< TVp.mag() << " GeV/c"<<G4endl;
  G4cout<<"Mass of Lam1663: "<< mass_lam1663 << " GeV/c2" <<G4endl;
  G4cout<<"################################################################" << G4endl;
  */
  LvLam1663.setVect(TVp);
  LvLam1663.setE(sqrt(mass_lam1663*mass_lam1663+TVp.mag2()));
  //LvLam1663.setE(sqrt(1.664*1.664+TVp.mag2()));

  EvtVector4R pInit_lam1663( LvLam1663.e(), LvLam1663.vect().x(), LvLam1663.vect().y(), LvLam1663.vect().z() );
  lam1663 = EvtParticleFactory::particleFactory(LAM1663, pInit_lam1663);
  GenerateDecay(anEvent, evtGenerator, lam1663, D);
}

void PrimaryGeneratorAction::GenerateTest45(G4Event* anEvent, EvtGen *evtGenerator, G4ThreeVector D, G4ThreeVector P, int C, int momentum_selection)
{
  ConfMan *confMan = ConfMan::GetConfManager();
  int crosssection = confMan->CrosssectionMode();

  //G4cout<<"Start Generate Test45"<<G4endl;
  /// mother particle momentum //
  double mass_pi = 0.13957061;
  double mass_proton = 0.938272081;
  G4LorentzVector lv_beam;
  G4LorentzVector lv_target;
  G4LorentzVector lv_particle;
  double pbeam = P.mag();

  lv_beam.setX(P.x());
  lv_beam.setY(P.y());
  lv_beam.setZ(P.z());
  lv_beam.setE(sqrt(mass_pi*mass_pi + pbeam*pbeam));

  lv_target.setX(0.0);
  lv_target.setY(0.0);
  lv_target.setZ(0.0);
  lv_target.setE(sqrt(mass_proton*mass_proton));

  lv_particle = lv_beam + lv_target;

  // make mother particle //
  EvtParticle* Nstar(0);
  EvtId evtid_N;
  if(C==-1){
    if(momentum_selection==0)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1460)0")); //p=0.635 GeV/c
    else if(momentum_selection==1)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1580)0")); //p=0.835 GeV/c
    else if(momentum_selection==2)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1690)0")); //p=1.035 GeV/c
    else if(momentum_selection==3)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1800)0")); //p=1.235 GeV/c
    else if(momentum_selection==4)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1900)0")); //p=1.435 GeV/c
    else if(momentum_selection==5)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1990)0")); //p=1.635 GeV/c
    else if(momentum_selection==6)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(2090)0")); //p=1.835 GeV/c
    else if(momentum_selection==7)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(2160)0")); //p=2.000 GeV/c

    else if(momentum_selection==8)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1460)0")); //p=0.635 GeV/c
    else if(momentum_selection==9)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1580)0")); //p=0.835 GeV/c
    else if(momentum_selection==10)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1690)0")); //p=1.035 GeV/c
    else if(momentum_selection==11)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1800)0")); //p=1.235 GeV/c
    else if(momentum_selection==12)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1900)0")); //p=1.435 GeV/c
    else if(momentum_selection==13)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1990)0")); //p=1.635 GeV/c
    else if(momentum_selection==14)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(2090)0")); //p=1.835 GeV/c
    else if(momentum_selection==15)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(2160)0")); //p=2.000 GeV/c

    else{
      G4cout<<"### No Particle data in param/EVT ###"<<G4endl;
      return;
    }
  }
  else if(C==1){
    if(momentum_selection==0)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1460)++")); //p=0.635 GeV/c
    else if(momentum_selection==1)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1580)++")); //p=0.835 GeV/c
    else if(momentum_selection==2)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1690)++")); //p=1.035 GeV/c
    else if(momentum_selection==3)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1800)++")); //p=1.235 GeV/c
    else if(momentum_selection==4)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1900)++")); //p=1.435 GeV/c
    else if(momentum_selection==5)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(1990)++")); //p=1.635 GeV/c
    else if(momentum_selection==6)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(2090)++")); //p=1.835 GeV/c
    else if(momentum_selection==7)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipip(2160)++")); //p=2.000 GeV/c

    else if(momentum_selection==8)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1460)++")); //p=0.635 GeV/c
    else if(momentum_selection==9)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1580)++")); //p=0.835 GeV/c
    else if(momentum_selection==10)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1690)++")); //p=1.035 GeV/c
    else if(momentum_selection==11)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1800)++")); //p=1.235 GeV/c
    else if(momentum_selection==12)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1900)++")); //p=1.435 GeV/c
    else if(momentum_selection==13)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(1990)++")); //p=1.635 GeV/c
    else if(momentum_selection==14)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(2090)++")); //p=1.835 GeV/c
    else if(momentum_selection==15)  evtid_N = EvtPDL::getId(std::string("PhaseSpacepipin(2160)++")); //p=2.000 GeV/c

    else{
      G4cout<<"### No Particle data in param/EVT ###"<<G4endl;
      return;
    }
  }
  else{
    G4cout<<"### Put a pion charge information ###"<<G4endl;
    return;
  }

  G4LorentzVector Lv_N;
  G4ThreeVector TVp (lv_particle.x(), lv_particle.y(), lv_particle.z());

  double sigmabvx, sigmabvy, sigmabvz;
  sigmabvx = confMan->GetBeamVXSig();
  sigmabvy = confMan->GetBeamVYSig();
  sigmabvz = confMan->GetBeamVZSig();

  int flag=1;
  double TV_bvx, TV_bvy, TV_bvz;
  while(flag){
    TRandom *eventgen = new TRandom(prmclass);
    TV_bvx = eventgen->Gaus(D.x(),sigmabvx);
    TV_bvy = eventgen->Gaus(D.y(),sigmabvy);
    TV_bvz = D.z() + eventgen->Uniform(-27.0,27.0);
    if(TMath::Sqrt(   (TV_bvx-D.x())*(TV_bvx-D.x()) + (TV_bvz-D.z())*(TV_bvz-D.z())  )< 54.0 && TMath::Abs(TV_bvy-D.y())<50.0)  flag=0;
    prmclass++;
  }

  G4ThreeVector TVx (TV_bvx,TV_bvy,TV_bvz);
  double mass_N = sqrt((lv_beam.e()+lv_target.e())*(lv_beam.e()+lv_target.e()) - pbeam*pbeam);

  // check total energy //
  if(mass_N < 0.938272081 + 2*0.13957061 ) //p, pi, pi mass sum
    {
      G4cout<<"### Beam momentum is not enough to generate Particle ###"<<G4endl;
      return;
    }
  /*
  G4cout<<"########################### Test  ##############################"<<G4endl;
  G4cout<<"Momentum of K-: "<<pbeam << " GeV/c" <<G4endl;
  G4cout<<"Invariant mass of K + p: "<<lv_particle.m() << " GeV/c2" <<G4endl;
  G4cout<<"Momentum of generated particle: "<< TVp.mag() << " GeV/c"<<G4endl;
  G4cout<<"Mass of generated particle: "<< mass_N << " GeV/c2" <<G4endl;
  G4cout<<"################################################################" << G4endl;
  */
  Lv_N.setVect(TVp);
  Lv_N.setE(sqrt(mass_N*mass_N+TVp.mag2()));

  EvtVector4R pInit_N( Lv_N.e(), Lv_N.vect().x(), Lv_N.vect().y(), Lv_N.vect().z() );
  Nstar = EvtParticleFactory::particleFactory(evtid_N, pInit_N);

  if(crosssection==1) GenerateDecay_angle(anEvent, evtGenerator, Nstar, P, TVx);
  else GenerateDecay(anEvent, evtGenerator, Nstar, TVx);
}

void PrimaryGeneratorAction::GenerateDecay(G4Event* anEvent, EvtGen *evtGenerator, EvtParticle* particle, G4ThreeVector D)
{


  static EvtStdHep evtstdhep;
  static EvtSecondary evtsecondary;

  EvtId        list_of_stable[10];
  EvtParticle* stable_parent[10];

  list_of_stable[0]=EvtId(-1,-1);
  stable_parent[0]=0;

  evtsecondary.init();
  evtstdhep.init();
  evtGenerator -> generateDecay(particle);
  particle->makeStdHep(evtstdhep,evtsecondary,list_of_stable);

  int npart = evtstdhep.getNPart();

  bool generate_flag = false;
  if(npart<100) generate_flag = true;
  while(!generate_flag)
    {
      G4cout<<"!!!Particles in EvtGen is more than 99."<<G4endl;
      evtsecondary.init();
      evtstdhep.init();
      evtGenerator -> generateDecay(particle);
      particle->makeStdHep(evtstdhep,evtsecondary,list_of_stable);
      if(evtstdhep.getNPart() < 100) generate_flag = true;
    }

  int j;
  int istat;
  int partnum;
  double px,py,pz,e,m;
  double x,y,z,t;

  EvtVector4R p4,x4;
  int n_beam = 0; // number of beams

  for(int i=0;i<evtstdhep.getNPart();i++)
    {
      j=i+1;
      int jmotherfirst=evtstdhep.getFirstMother(i)+1;
      int jmotherlast=evtstdhep.getLastMother(i)+1;
      int jdaugfirst=evtstdhep.getFirstDaughter(i)+1;
      int jdauglast=evtstdhep.getLastDaughter(i)+1;

      partnum=evtstdhep.getStdHepID(i);

      istat=evtstdhep.getIStat(i);

      p4=evtstdhep.getP4(i);
      x4=evtstdhep.getX4(i);

      px=p4.get(1);
      py=p4.get(2);
      pz=p4.get(3);
      e=p4.get(0);

      x=x4.get(1)+D.x();
      y=x4.get(2)+D.y();
      z=x4.get(3)+D.z();
      //t=x4.get(0)+D.t();
      t=x4.get(0)*0.001/2.999792458 *1.e1; //mm/c --> ns
      m=p4.mass();

      EvtVector4R evx4, evp4;
      evx4.set(1, x);
      evx4.set(2, y);
      evx4.set(3, z);
      evx4.set(0, t);

      evp4.set(1, px);
      evp4.set(2, py);
      evp4.set(3, pz);
      evp4.set(0, e);

      bool beam_flag = false;
      int tr_id = -1;
      //if(jdaugfirst ==0 && jdauglast ==0 && (partnum == 2212 || partnum == -211) && jmotherfirst == 2)
      if(jdaugfirst ==0 && jdauglast ==0)
	{
	  makeGun(anEvent, partnum, evx4, evp4);
	  beam_flag = true;
	  n_beam++;
	}
      if(beam_flag == true)
	{
	  tr_id = n_beam;
	}
      anaMan_->SetEvtGen(j, partnum, jmotherfirst, jmotherlast, jdaugfirst, jdauglast, tr_id, evx4, evp4);
    }

#if 0
  G4cout<<"############# Particle decay table  ##############"<<G4endl;
  G4cout<<"Npart: "<<npart<<G4endl;
  for(int i=0;i<evtstdhep.getNPart();i++)
    {
      j=i+1;
      int jmotherfirst=evtstdhep.getFirstMother(i)+1;
      int jmotherlast=evtstdhep.getLastMother(i)+1;
      int jdaugfirst=evtstdhep.getFirstDaughter(i)+1;
      int jdauglast=evtstdhep.getLastDaughter(i)+1;

      p4=evtstdhep.getP4(i);
      x4=evtstdhep.getX4(i);

      px=p4.get(1);
      py=p4.get(2);
      pz=p4.get(3);
      e=p4.get(0);

      x=x4.get(1)+D.x();
      y=x4.get(2)+D.y();
      z=x4.get(3)+D.z();
      t=x4.get(0);
      m=p4.mass();
      G4cout<<"x : "<<x<<" y " <<y<<" z: "<<z<<G4endl;

      partnum=evtstdhep.getStdHepID(i);
      G4cout<<"ID: " << j<< "  Particle Num: "<<partnum<<"  mf: "<<jmotherfirst<< "  ml: "<<jmotherlast << "  df: "<<jdaugfirst << "  dl: "<<jdauglast<<G4endl;
      G4cout<< "   p: "<<(float)sqrt(px*px+py*py+pz*pz) << " e: " << (float)e << " t: "<< (float)t<< " m: "<< (float)m <<G4endl;
    }

  G4cout<<"##################################################"<<G4endl;
#endif
  particle->deleteTree();

}

void PrimaryGeneratorAction::makeGun(G4Event* anEvent, int partnum, EvtVector4R x4, EvtVector4R p4)
{
  particleGun -> SetParticleDefinition (particleTable -> FindParticle(partnum));
  G4ThreeVector beamx ( x4.get(1), x4.get(2), x4.get(3));
  G4ThreeVector beamp ( p4.get(1), p4.get(2), p4.get(3));
  G4ThreeVector beampu = beamp/beamp.mag();
  G4double energy = (p4.get(0) - p4.mass())*GeV;
  //G4double energy = beamp.mag()*GeV;
  double time = x4.get(0);
  G4double g4time =  time * ns;

  //std::cout<<"Beam energy: "<< energy << std::endl;
  //G4cout<<"###############  beam generated  #################" << G4endl;
  //G4cout<<"particle: "<<partnum<< " vertex(x,y,z) ("<< beamx.x() << ", " << beamx.y() << ", " << beamx.z() << ") "
  //	<< "energy: " <<energy/GeV<< " momentum(x,y,z) ("<< p4.get(1) << ", " << p4.get(2) << ", " <<p4.get(3) << ") "<< beamp.mag()<< " time: "<<time
  //	<< G4endl;
  //G4cout<<"##################################################" << G4endl;
  particleGun->SetParticleMomentumDirection ( beampu );
  //particleGun->SetParticleMomentum ( beamp );
  particleGun->SetParticleTime ( g4time );
  particleGun->SetParticlePosition( beamx );
  particleGun->SetParticleEnergy( energy );
  particleGun->GeneratePrimaryVertex( anEvent);

}

void PrimaryGeneratorAction::GenerateDecay_angle(G4Event* anEvent, EvtGen *evtGenerator, EvtParticle* particle, G4ThreeVector P, G4ThreeVector D)
{

  G4cout<<"angle"<<G4endl;

  ConfMan *confMan = ConfMan::GetConfManager();
  double coefficient[10];
  for(int i=0;i<10;i++){
    coefficient[i] = confMan->GetLegendre(i);
    G4cout<<"coefficient "<< i<<" th : "<<coefficient[i]<<G4endl;
  }

  TF1 *legen[10];
  legen[0] = new TF1("legen[0]","1",-1,1);
  legen[1] = new TF1("legen[1]","x",-1,1);
  legen[2] = new TF1("legen[2]","(3*TMath::Power(x,2)-1)/2 ",-1,1);
  legen[3] = new TF1("legen[3]","(5*TMath::Power(x,3)-3*x)/2 ",-1,1);
  legen[4] = new TF1("legen[4]","(35*TMath::Power(x,4)-30*TMath::Power(x,2)+3)/8 ",-1,1);
  legen[5] = new TF1("legen[5]","(63*TMath::Power(x,5)-70*TMath::Power(x,3)+15*x)/8 ",-1,1);
  legen[6] = new TF1("legen[6]","(231*TMath::Power(x,6)-315*TMath::Power(x,4)+105*TMath::Power(x,2)-5)/16 ",-1,1);
  legen[7] = new TF1("legen[7]","(429*TMath::Power(x,7)-693*TMath::Power(x,5)+315*TMath::Power(x,3)-35*x)/16 ",-1,1);
  legen[8] = new TF1("legen[8]","(6435*TMath::Power(x,8)-12012*TMath::Power(x,6)+6930*TMath::Power(x,4)-1260*TMath::Power(x,2)+35)/128 ",-1,1);
  legen[9] = new TF1("legen[9]","(12155*TMath::Power(x,9)-25740*TMath::Power(x,7)+18018*TMath::Power(x,5)-4620*TMath::Power(x,3)+315*x)/128 ",-1,1);

  TF1 *func = new TF1("diff",Form("legen[0]*%f+legen[1]*%f+legen[2]*%f+legen[3]*%f+legen[4]*%f+legen[5]*%f+legen[6]*%f+legen[7]*%f+legen[8]*%f+legen[9]*%f",coefficient[0],coefficient[1],coefficient[2],coefficient[3],coefficient[4],coefficient[5],coefficient[6],coefficient[7],coefficient[8],coefficient[9]),-1.0,1.0);

  double func_max = func->GetMaximum();
  double m_p = 0.938272;
  double m_pi = 0.139570;
  double p_beam = P.mag();
  double e_beam = sqrt(p_beam*p_beam + m_pi*m_pi);
  double cos_CM;

  TVector3 beta_CM(0,0,-p_beam/(e_beam+m_p));
  EvtVector4R p4_pi;

  static EvtStdHep evtstdhep;
  static EvtSecondary evtsecondary;

  EvtId        list_of_stable[10];
  EvtParticle* stable_parent[10];
  list_of_stable[0]=EvtId(-1,-1);
  stable_parent[0]=0;

  int flag=1;
  while(flag){
    evtsecondary.init();
    evtstdhep.init();
    evtGenerator -> generateDecay(particle);
    particle->makeStdHep(evtstdhep,evtsecondary,list_of_stable);
    int npart = evtstdhep.getNPart();

    for(int i=0;i<npart;i++){
      if(TMath::Abs(evtstdhep.getStdHepID(i))==211){
	p4_pi=evtstdhep.getP4(i);
	TLorentzVector lv_pi(p4_pi.get(1),p4_pi.get(2),p4_pi.get(3),p4_pi.get(0));
	TLorentzVector lv_beam(P.x(),P.y(),P.z(),e_beam);
	lv_pi.Boost(beta_CM);
	lv_beam.Boost(beta_CM);
	cos_CM = GetCos(lv_beam,lv_pi);
      }
    }
    TRandom *eventgen2 = new TRandom(prmclass2);
    if((eventgen2->Uniform(0,1))*(func_max) < (func->Eval(cos_CM)) && npart<100) flag=0;
    prmclass2++;
  }

  int j;
  int istat;
  int partnum;
  double px,py,pz,e,m;
  double x,y,z,t;

  EvtVector4R p4,x4;
  int n_beam = 0; // number of beams

  for(int i=0;i<evtstdhep.getNPart();i++){
    j=i+1;
    int jmotherfirst=evtstdhep.getFirstMother(i)+1;
    int jmotherlast=evtstdhep.getLastMother(i)+1;
    int jdaugfirst=evtstdhep.getFirstDaughter(i)+1;
    int jdauglast=evtstdhep.getLastDaughter(i)+1;

    partnum=evtstdhep.getStdHepID(i);

    istat=evtstdhep.getIStat(i);

    p4=evtstdhep.getP4(i);
    x4=evtstdhep.getX4(i);

    px=p4.get(1);
    py=p4.get(2);
    pz=p4.get(3);
    e=p4.get(0);

    x=x4.get(1)+D.x();
    y=x4.get(2)+D.y();
    z=x4.get(3)+D.z();
    t=x4.get(0)*0.001/2.999792458 *1.e1; //mm/c --> ns
    m=p4.mass();

    EvtVector4R evx4, evp4;
    evx4.set(1, x);
    evx4.set(2, y);
    evx4.set(3, z);
    evx4.set(0, t);

    evp4.set(1, px);
    evp4.set(2, py);
    evp4.set(3, pz);
    evp4.set(0, e);

    bool beam_flag = false;
    int tr_id = -1;
    //if(jdaugfirst ==0 && jdauglast ==0 && (partnum == 2212 || partnum == -211) && jmotherfirst == 2)
    if(jdaugfirst ==0 && jdauglast ==0){
      makeGun(anEvent, partnum, evx4, evp4);
      beam_flag = true;
      n_beam++;
    }
    if(beam_flag == true){
      tr_id = n_beam;
    }
    anaMan_->SetEvtGen(j, partnum, jmotherfirst, jmotherlast, jdaugfirst, jdauglast, tr_id, evx4, evp4);
  }

#if 1
  G4cout<<"############# Particle decay table  ##############"<<G4endl;
  G4cout<<"Npart: "<<npart<<G4endl;
  for(int i=0;i<evtstdhep.getNPart();i++)
    {
      j=i+1;
      int jmotherfirst=evtstdhep.getFirstMother(i)+1;
      int jmotherlast=evtstdhep.getLastMother(i)+1;
      int jdaugfirst=evtstdhep.getFirstDaughter(i)+1;
      int jdauglast=evtstdhep.getLastDaughter(i)+1;

      p4=evtstdhep.getP4(i);
      x4=evtstdhep.getX4(i);

      px=p4.get(1);
      py=p4.get(2);
      pz=p4.get(3);
      e=p4.get(0);

      x=x4.get(1)+D.x();
      y=x4.get(2)+D.y();
      z=x4.get(3)+D.z();
      t=x4.get(0);
      m=p4.mass();
      G4cout<<"x : "<<x<<" y " <<y<<" z: "<<z<<G4endl;

      partnum=evtstdhep.getStdHepID(i);
      G4cout<<"ID: " << j<< "  Particle Num: "<<partnum<<"  mf: "<<jmotherfirst<< "  ml: "<<jmotherlast << "  df: "<<jdaugfirst << "  dl: "<<jdauglast<<G4endl;
      G4cout<< "   p: "<<(float)sqrt(px*px+py*py+pz*pz) << " e: " << (float)e << " t: "<< (float)t<< " m: "<< (float)m <<G4endl;
    }

  G4cout<<"##################################################"<<G4endl;
#endif
  particle->deleteTree();

}

double PrimaryGeneratorAction::GetCos(TLorentzVector lv1, TLorentzVector lv2) {
  double dot_product = lv1.Px()*lv2.Px() + lv1.Py()*lv2.Py() + lv1.Pz()*lv2.Pz();
  double cos = dot_product / (lv1.P() * lv2.P());
  return cos;
}
