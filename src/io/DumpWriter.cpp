/*
 * Copyright (c) 2005 The University of Notre Dame. All Rights Reserved.
 *
 * The University of Notre Dame grants you ("Licensee") a
 * non-exclusive, royalty free, license to use, modify and
 * redistribute this software in source and binary code form, provided
 * that the following conditions are met:
 *
 * 1. Acknowledgement of the program authors must be made in any
 *    publication of scientific results based in part on use of the
 *    program.  An acceptable form of acknowledgement is citation of
 *    the article in which the program was described (Matthew
 *    A. Meineke, Charles F. Vardeman II, Teng Lin, Christopher
 *    J. Fennell and J. Daniel Gezelter, "OOPSE: An Object-Oriented
 *    Parallel Simulation Engine for Molecular Dynamics,"
 *    J. Comput. Chem. 26, pp. 252-271 (2005))
 *
 * 2. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 3. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * This software is provided "AS IS," without a warranty of any
 * kind. All express or implied conditions, representations and
 * warranties, including any implied warranty of merchantability,
 * fitness for a particular purpose or non-infringement, are hereby
 * excluded.  The University of Notre Dame and its licensors shall not
 * be liable for any damages suffered by licensee as a result of
 * using, modifying or distributing the software or its
 * derivatives. In no event will the University of Notre Dame or its
 * licensors be liable for any lost revenue, profit or data, or for
 * direct, indirect, special, consequential, incidental or punitive
 * damages, however caused and regardless of the theory of liability,
 * arising out of the use of or inability to use software, even if the
 * University of Notre Dame has been advised of the possibility of
 * such damages.
 */
 
#include "io/DumpWriter.hpp"
#include "primitives/Molecule.hpp"
#include "utils/simError.h"
#include "io/basic_teebuf.hpp"
#include "io/gzstream.hpp"
#include "io/Globals.hpp"

#ifdef IS_MPI
#include <mpi.h>
#endif //is_mpi

namespace oopse {

  DumpWriter::DumpWriter(SimInfo* info) 
    : info_(info), filename_(info->getDumpFileName()), eorFilename_(info->getFinalConfigFileName()){

    Globals* simParams = info->getSimParams();
    needCompression_ = simParams->getCompressDumpFile();
    needForceVector_ = simParams->getOutputForceVector();
    createDumpFile_ = true;
#ifdef HAVE_LIBZ
    if (needCompression_) {
      filename_ += ".gz";
      eorFilename_ += ".gz";
    }
#endif
    
#ifdef IS_MPI

    if (worldRank == 0) {
#endif // is_mpi
        
      dumpFile_ = createOStream(filename_);

      if (!dumpFile_) {
        sprintf(painCave.errMsg, "Could not open \"%s\" for dump output.\n",
                filename_.c_str());
        painCave.isFatal = 1;
        simError();
      }

#ifdef IS_MPI

    }

#endif // is_mpi

  }


  DumpWriter::DumpWriter(SimInfo* info, const std::string& filename) 
    : info_(info), filename_(filename){

    Globals* simParams = info->getSimParams();
    eorFilename_ = filename_.substr(0, filename_.rfind(".")) + ".eor";    

    needCompression_ = simParams->getCompressDumpFile();
    needForceVector_ = simParams->getOutputForceVector();
    createDumpFile_ = true;
#ifdef HAVE_LIBZ
    if (needCompression_) {
      filename_ += ".gz";
      eorFilename_ += ".gz";
    }
#endif
    
#ifdef IS_MPI

    if (worldRank == 0) {
#endif // is_mpi

      
      dumpFile_ = createOStream(filename_);

      if (!dumpFile_) {
        sprintf(painCave.errMsg, "Could not open \"%s\" for dump output.\n",
                filename_.c_str());
        painCave.isFatal = 1;
        simError();
      }

#ifdef IS_MPI

    }

#endif // is_mpi

  }
  
  DumpWriter::DumpWriter(SimInfo* info, const std::string& filename, bool writeDumpFile) 
    : info_(info), filename_(filename){
    
    Globals* simParams = info->getSimParams();
    eorFilename_ = filename_.substr(0, filename_.rfind(".")) + ".eor";    
    
    needCompression_ = simParams->getCompressDumpFile();
    needForceVector_ = simParams->getOutputForceVector();
    
#ifdef HAVE_LIBZ
    if (needCompression_) {
      filename_ += ".gz";
      eorFilename_ += ".gz";
    }
#endif
    
#ifdef IS_MPI
    
    if (worldRank == 0) {
#endif // is_mpi
      
      createDumpFile_ = writeDumpFile;
      if (createDumpFile_) {
        dumpFile_ = createOStream(filename_);
      
        if (!dumpFile_) {
          sprintf(painCave.errMsg, "Could not open \"%s\" for dump output.\n",
                  filename_.c_str());
          painCave.isFatal = 1;
          simError();
        }
      }
#ifdef IS_MPI
      
    }

    
#endif // is_mpi
    
  }

  DumpWriter::~DumpWriter() {

#ifdef IS_MPI

    if (worldRank == 0) {
#endif // is_mpi
      if (createDumpFile_){
        writeClosing(*dumpFile_);
        delete dumpFile_;
      }
#ifdef IS_MPI

    }

#endif // is_mpi

  }

  void DumpWriter::writeFrameProperties(std::ostream& os, Snapshot* s) {

    char buffer[1024];

    os << "    <FrameData>\n";

    RealType currentTime = s->getTime();
    sprintf(buffer, "        Time: %.10g\n", currentTime);
    os << buffer;

    Mat3x3d hmat;
    hmat = s->getHmat();
    sprintf(buffer, "        Hmat: {{ %.10g, %.10g, %.10g }, { %.10g, %.10g, %.10g }, { %.10g, %.10g, %.10g }}\n", 
            hmat(0, 0), hmat(1, 0), hmat(2, 0), 
            hmat(0, 1), hmat(1, 1), hmat(2, 1),
            hmat(0, 2), hmat(1, 2), hmat(2, 2));
    os << buffer;

    RealType chi = s->getChi();
    RealType integralOfChiDt = s->getIntegralOfChiDt();
    sprintf(buffer, "  Thermostat: %.10g , %.10g\n", chi, integralOfChiDt);
    os << buffer;

    Mat3x3d eta;
    eta = s->getEta();
    sprintf(buffer, "    Barostat: {{ %.10g, %.10g, %.10g }, { %.10g, %.10g, %.10g }, { %.10g, %.10g, %.10g }}\n",
            eta(0, 0), eta(1, 0), eta(2, 0), 
            eta(0, 1), eta(1, 1), eta(2, 1),
            eta(0, 2), eta(1, 2), eta(2, 2));
    os << buffer;

    os << "    </FrameData>\n";
  }

  void DumpWriter::writeFrame(std::ostream& os) {

#ifdef IS_MPI
    MPI_Status istatus;
#endif

    Molecule* mol;
    StuntDouble* integrableObject;
    SimInfo::MoleculeIterator mi;
    Molecule::IntegrableObjectIterator ii;

#ifndef IS_MPI
    os << "  <Snapshot>\n";
 
    writeFrameProperties(os, info_->getSnapshotManager()->getCurrentSnapshot());

    os << "    <StuntDoubles>\n";
    for (mol = info_->beginMolecule(mi); mol != NULL; mol = info_->nextMolecule(mi)) {

      for (integrableObject = mol->beginIntegrableObject(ii); integrableObject != NULL; 
           integrableObject = mol->nextIntegrableObject(ii)) { 	
        os << prepareDumpLine(integrableObject);

      }
    }    
    os << "    </StuntDoubles>\n";

    os << "  </Snapshot>\n";

    os.flush();
#else
    //every node prepares the dump lines for integrable objects belong to itself
    std::string buffer;
    for (mol = info_->beginMolecule(mi); mol != NULL; mol = info_->nextMolecule(mi)) {
      
      for (integrableObject = mol->beginIntegrableObject(ii); integrableObject != NULL; 
           integrableObject = mol->nextIntegrableObject(ii)) { 	
        buffer += prepareDumpLine(integrableObject);
      }
    }
    
    const int masterNode = 0;

    if (worldRank == masterNode) {	
      os << "  <Snapshot>\n";	
      writeFrameProperties(os, info_->getSnapshotManager()->getCurrentSnapshot());
      os << "    <StuntDoubles>\n";
	
      os << buffer;

      int nProc;
      MPI_Comm_size(MPI_COMM_WORLD, &nProc);
      for (int i = 1; i < nProc; ++i) {

        // receive the length of the string buffer that was
        // prepared by processor i

        int recvLength;
        MPI_Recv(&recvLength, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &istatus);
        char* recvBuffer = new char[recvLength];
        if (recvBuffer == NULL) {
        } else {
          MPI_Recv(recvBuffer, recvLength, MPI_CHAR, i, 0, MPI_COMM_WORLD, &istatus);
          os << recvBuffer;
          delete recvBuffer;
        }
      }	
      os << "    </StuntDoubles>\n";
      
      os << "  </Snapshot>\n";
      os.flush();
    } else {
      int sendBufferLength = buffer.size() + 1;
      MPI_Send(&sendBufferLength, 1, MPI_INT, masterNode, 0, MPI_COMM_WORLD);
      MPI_Send((void *)buffer.c_str(), sendBufferLength, MPI_CHAR, masterNode, 0, MPI_COMM_WORLD);
    }

#endif // is_mpi

  }

  std::string DumpWriter::prepareDumpLine(StuntDouble* integrableObject) {
	
    int index = integrableObject->getGlobalIntegrableObjectIndex();
    std::string type("pv");
    std::string line;
    char tempBuffer[4096];

    Vector3d pos;
    Vector3d vel;
    pos = integrableObject->getPos();
    vel = integrableObject->getVel();		
    sprintf(tempBuffer, "%18.10g %18.10g %18.10g %13e %13e %13e", 
            pos[0], pos[1], pos[2],
            vel[0], vel[1], vel[2]);		        
    line += tempBuffer;

    if (integrableObject->isDirectional()) {
      type += "qj";
      Quat4d q;
      Vector3d ji;
      q = integrableObject->getQ();
      ji = integrableObject->getJ();
      sprintf(tempBuffer, " %13e %13e %13e %13e %13e %13e %13e",
              q[0], q[1], q[2], q[3],
              ji[0], ji[1], ji[2]);
      line += tempBuffer;
    }

    if (needForceVector_) {
      type += "ft";
      Vector3d frc;
      Vector3d trq;
      frc = integrableObject->getFrc();
      trq = integrableObject->getTrq();
              
      sprintf(tempBuffer, " %13e %13e %13e %13e %13e %13e",
              frc[0], frc[1], frc[2],
              trq[0], trq[1], trq[2]);
      line += tempBuffer;
    }
	
    sprintf(tempBuffer, "%10d %7s %s\n", index, type.c_str(), line.c_str());
    return std::string(tempBuffer);
  }

  void DumpWriter::writeDump() {
    writeFrame(*dumpFile_);
  }

  void DumpWriter::writeEor() {
    std::ostream* eorStream;
    
#ifdef IS_MPI
    if (worldRank == 0) {
#endif // is_mpi

      eorStream = createOStream(eorFilename_);

#ifdef IS_MPI
    }
#endif // is_mpi    

    writeFrame(*eorStream);

#ifdef IS_MPI
    if (worldRank == 0) {
#endif // is_mpi
      writeClosing(*eorStream);
      delete eorStream;
#ifdef IS_MPI
    }
#endif // is_mpi  

  }


  void DumpWriter::writeDumpAndEor() {
    std::vector<std::streambuf*> buffers;
    std::ostream* eorStream;
#ifdef IS_MPI
    if (worldRank == 0) {
#endif // is_mpi

      buffers.push_back(dumpFile_->rdbuf());

      eorStream = createOStream(eorFilename_);

      buffers.push_back(eorStream->rdbuf());
        
#ifdef IS_MPI
    }
#endif // is_mpi    

    TeeBuf tbuf(buffers.begin(), buffers.end());
    std::ostream os(&tbuf);

    writeFrame(os);

#ifdef IS_MPI
    if (worldRank == 0) {
#endif // is_mpi
      writeClosing(*eorStream);
      delete eorStream;
#ifdef IS_MPI
    }
#endif // is_mpi  
    
  }

  std::ostream* DumpWriter::createOStream(const std::string& filename) {

    std::ostream* newOStream;
#ifdef HAVE_LIBZ 
    if (needCompression_) {
      newOStream = new ogzstream(filename.c_str());
    } else {
      newOStream = new std::ofstream(filename.c_str());
    }
#else
    newOStream = new std::ofstream(filename.c_str());
#endif
    //write out MetaData first
    (*newOStream) << "<OOPSE version=4>" << std::endl;
    (*newOStream) << "  <MetaData>" << std::endl;
    (*newOStream) << info_->getRawMetaData();
    (*newOStream) << "  </MetaData>" << std::endl;
    return newOStream;
  }

  void DumpWriter::writeClosing(std::ostream& os) {

    os << "</OOPSE>\n";
    os.flush();
  }

}//end namespace oopse
