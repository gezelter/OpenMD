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
 
/**
 * @file RigidBody.hpp
 * @author    tlin
 * @date  10/23/2004
 * @version 1.0
 */ 

#ifndef PRIMITIVES_RIGIDBODY_HPP
#define PRIMITIVES_RIGIDBODY_HPP

#include <vector>

#include "primitives/StuntDouble.hpp"
#include "primitives/DirectionalAtom.hpp"
#include "types/AtomStamp.hpp"
namespace oopse{
class RigidBody : public StuntDouble {
    public:

        typedef std::vector<Atom*>::iterator AtomIterator;        

        RigidBody();

        virtual std::string getType() { return name_;}
        
        /** Sets the name of this stuntdouble*/
        virtual void setType(const std::string& name) { name_ = name;}
    

       /**
         * Sets  the previous rotation matrix of this stuntdouble
         * @param a  new rotation matrix 
         */         
       virtual void setPrevA(const RotMat3x3d& a);
       
       /**
         * Sets  the current rotation matrix of this stuntdouble
         * @param a  new rotation matrix 
         * @note setA will not change the position and rotation matrix of Directional atoms belong to
         * this rigidbody. If you want to do that, use #updateAtoms
         */         
        virtual void setA(const RotMat3x3d& a);
       /**
         * Sets  the rotation matrix of this stuntdouble in specified snapshot
         * @param a rotation matrix to be set 
         * @param snapshotNo 
         * @see #getA
         */         
        virtual void setA(const RotMat3x3d& a, int snapshotNo);

        /**
         * Returns the inertia tensor of this stuntdouble
         * @return the inertia tensor of this stuntdouble
         */ 
        virtual Mat3x3d getI();


        /** Sets the internal unit frame of this stuntdouble by three euler angles */
        void setElectroFrameFromEuler(double phi, double theta, double psi);
        
        /**
         * Returns the gradient of this stuntdouble
         * @return the inertia tensor of this stuntdouble
         * @see #setI
         */ 
        virtual std::vector<double> getGrad();

        virtual void accept(BaseVisitor* v);

        void addAtom(Atom* at, AtomStamp* ats);

        /** calculate the reference coordinates */
        void calcRefCoords();

        /** Convert Atomic forces and torques to total forces and torques */
        void calcForcesAndTorques();

        /** update the positions of atoms belong to this rigidbody */
        void updateAtoms();

        Atom* beginAtom(std::vector<Atom*>::iterator& i);

        Atom* nextAtom(std::vector<Atom*>::iterator& i);

        std::vector<Atom*>::iterator getBeginAtomIter() {
            return atoms_.begin();
        }
        
        std::vector<Atom*>::iterator getEndAtomIter() {
            return atoms_.end();
        }

        /** 
         * Returns the atoms of this rigid body
         * @return the atoms of this rigid body in a vector
         * @deprecate
         */           
        std::vector<Atom*> getAtoms() {
            return atoms_;
        }

        /** 
         * Returns the number of atoms in this rigid body
         * @return the number of atoms in this rigid body
         */
        int getNumAtoms() {
            return atoms_.size();
        }

        /**
         * Return the position of atom which belongs to this rigid body.
         * @return true if index is valid otherwise return false
         * @param pos the position of atom which will be set on return if index is valid
         * @param index the index of the atom in rigid body's private data member atoms_
         */
        bool getAtomPos(Vector3d& pos, unsigned int index);

        /**
         * Return the position of atom which belongs to this rigid body.
         * @return true if atom belongs to this rigid body,otherwise return false
         * @param pos position of atom which will be set on return if atom belongs to this rigid body
         * @param atom the pointer to an atom
         */            
        bool getAtomPos(Vector3d& pos, Atom* atom);

        /**
         * Return the velocity of atom which belongs to this rigid body.
         * @return true if index is valid otherwise return false
         * @param vel the velocity of atom which will be set on return if index is valid
         * @param index the index of the atom in rigid body's private data member atoms_
         */
        bool getAtomVel(Vector3d& vel, unsigned int index);

        /**
         * Return the velocity of atom which belongs to this rigid body.
         * @return true if atom belongs to this rigid body,otherwise return false
         * @param vel velocity of atom which will be set on return if atom belongs to this rigid body
         * @param atom the pointer to an atom
         */ 
        bool getAtomVel(Vector3d& vel, Atom*);

        /**
         * Return the reference coordinate of atom which belongs to this rigid body.
         * @return true if index is valid otherwise return false
         * @param coor the reference coordinate of atom which will be set on return if index is valid
         * @param index the index of the atom in rigid body's private data member atoms_
         */
        bool getAtomRefCoor(Vector3d& coor, unsigned int index);

        /**
         * Return the velocity of atom which belongs to this rigid body.
         * @return true if atom belongs to this rigid body,otherwise return false
         * @param coor velocity of atom which will be set on return if atom belongs to this rigid body
         * @param atom the pointer to an atom
         */ 
        bool getAtomRefCoor(Vector3d& coor, Atom* atom);

    private:
        std::string name_;        
        Mat3x3d inertiaTensor_;     
        RotMat3x3d sU_;               /**< body fixed standard unit vector */
        
        std::vector<Atom*> atoms_;
        std::vector<Vector3d> refCoords_;
        std::vector<RotMat3x3d> refOrients_;
};

}//namepace oopse

#endif //PRIMITIVES_RIGIDBODY_HPP

