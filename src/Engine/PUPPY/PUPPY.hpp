#ifndef NITE_PUPPY_HPP
    #define NITE_PUPPY_HPP


    namespace nite {

        namespace Puppy {



            struct Base {
                nite::Batch batch;
                virtual void computeSize(){}
                virtual void step(){}
                virtual void render(){}
                
            };


        }

    }


#endif