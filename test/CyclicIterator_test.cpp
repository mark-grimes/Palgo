#include <palgo/impl/CyclicIterator.hpp>
#include "catch.hpp"


SCENARIO( "Check that a CyclicIterator works as expected" )
{
	GIVEN( "A CyclicIterator to a vector of ints" )
	{
		std::vector<int> input(4);
		// Fill with 0,1,2,3
		size_t generator=0;
		std::generate( input.begin(), input.end(), [&](){return generator++;} );

		palgo::impl::CyclicIterator<std::vector<int>::const_iterator> iCyclic( input.begin(), input.end() );

		WHEN( "Iterating forward" )
		{
			CHECK( *iCyclic==0 ); ++iCyclic;
			CHECK( *iCyclic==1 ); ++iCyclic;
			CHECK( *iCyclic==2 ); ++iCyclic;
			CHECK( *iCyclic==3 ); ++iCyclic;
			CHECK( *iCyclic==0 ); ++iCyclic;
			CHECK( *iCyclic==1 ); ++iCyclic;
			CHECK( *iCyclic==2 ); ++iCyclic;
			CHECK( *iCyclic==3 ); ++iCyclic;
			CHECK( *iCyclic==0 );
		}
		WHEN( "Iterating forwards with the post increment operator" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(iCyclic++)==0 );
			CHECK( *(iCyclic++)==1 );
			CHECK( *(iCyclic++)==2 );
			CHECK( *(iCyclic++)==3 );
			CHECK( *(iCyclic++)==0 );
			CHECK( *(iCyclic++)==1 );
			CHECK( *(iCyclic++)==2 );
			CHECK( *(iCyclic++)==3 );
		}
		WHEN( "Iterating forwards with the pre increment operator" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(++iCyclic)==1 );
			CHECK( *(++iCyclic)==2 );
			CHECK( *(++iCyclic)==3 );
			CHECK( *(++iCyclic)==0 );
			CHECK( *(++iCyclic)==1 );
			CHECK( *(++iCyclic)==2 );
			CHECK( *(++iCyclic)==3 );
			CHECK( *(++iCyclic)==0 );
		}
		WHEN( "Iterating backwards" )
		{
			CHECK( *iCyclic==0 ); --iCyclic;
			CHECK( *iCyclic==3 ); --iCyclic;
			CHECK( *iCyclic==2 ); --iCyclic;
			CHECK( *iCyclic==1 ); --iCyclic;
			CHECK( *iCyclic==0 ); --iCyclic;
			CHECK( *iCyclic==3 ); --iCyclic;
			CHECK( *iCyclic==2 ); --iCyclic;
			CHECK( *iCyclic==1 ); --iCyclic;
			CHECK( *iCyclic==0 );
		}
		WHEN( "Iterating backwards with the post decrement operator" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(iCyclic--)==0 );
			CHECK( *(iCyclic--)==3 );
			CHECK( *(iCyclic--)==2 );
			CHECK( *(iCyclic--)==1 );
			CHECK( *(iCyclic--)==0 );
			CHECK( *(iCyclic--)==3 );
			CHECK( *(iCyclic--)==2 );
			CHECK( *(iCyclic--)==1 );
		}
		WHEN( "Iterating backwards with the pre decrement operator" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(--iCyclic)==3 );
			CHECK( *(--iCyclic)==2 );
			CHECK( *(--iCyclic)==1 );
			CHECK( *(--iCyclic)==0 );
			CHECK( *(--iCyclic)==3 );
			CHECK( *(--iCyclic)==2 );
			CHECK( *(--iCyclic)==1 );
			CHECK( *(--iCyclic)==0 );
		}
		WHEN( "Adding offsets" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(iCyclic+1)==1 );
			CHECK( *(iCyclic+2)==2 );
			CHECK( *(iCyclic+3)==3 );
			CHECK( *(iCyclic+4)==0 );
			CHECK( *(iCyclic+5)==1 );
			CHECK( *(iCyclic+6)==2 );
			CHECK( *(iCyclic+7)==3 );
			CHECK( *(iCyclic+8)==0 );
			CHECK( *(iCyclic+9)==1 );
			CHECK( *(iCyclic+10)==2 );
			CHECK( *(iCyclic+11)==3 );
			CHECK( *(iCyclic+12)==0 );
			CHECK( *(iCyclic+13)==1 );
			CHECK( *(iCyclic+14)==2 );
		}
		WHEN( "Adding and assigning offsets" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(iCyclic+=1)==1 );
			CHECK( *(iCyclic+=1)==2 );
			CHECK( *(iCyclic+=1)==3 );
			CHECK( *(iCyclic+=2)==1 );
			CHECK( *(iCyclic+=2)==3 );
			CHECK( *(iCyclic+=2)==1 );
			CHECK( *(iCyclic+=3)==0 );
			CHECK( *(iCyclic+=3)==3 );
			CHECK( *(iCyclic+=3)==2 );
			CHECK( *(iCyclic+=4)==2 );
			CHECK( *(iCyclic+=4)==2 );
			CHECK( *(iCyclic+=5)==3 );
			CHECK( *(iCyclic+=5)==0 );
			CHECK( *(iCyclic+=5)==1 );
			CHECK( *(iCyclic+=5)==2 );
			CHECK( *(iCyclic+=10)==0 );
			CHECK( *(iCyclic+=10)==2 );
			CHECK( *(iCyclic+=10)==0 );
		}
		WHEN( "Subtracting offsets" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(iCyclic-1)==3 );
			CHECK( *(iCyclic-2)==2 );
			CHECK( *(iCyclic-3)==1 );
			CHECK( *(iCyclic-4)==0 );
			CHECK( *(iCyclic-5)==3 );
			CHECK( *(iCyclic-6)==2 );
			CHECK( *(iCyclic-7)==1 );
			CHECK( *(iCyclic-8)==0 );
			CHECK( *(iCyclic-9)==3 );
		}
		WHEN( "Subtracting and assigning offsets" )
		{
			CHECK( *iCyclic==0 );
			CHECK( *(iCyclic-=1)==3 );
			CHECK( *(iCyclic-=1)==2 );
			CHECK( *(iCyclic-=1)==1 );
			CHECK( *(iCyclic-=1)==0 );
			CHECK( *(iCyclic-=2)==2 );
			CHECK( *(iCyclic-=2)==0 );
			CHECK( *(iCyclic-=3)==1 );
			CHECK( *(iCyclic-=3)==2 );
			CHECK( *(iCyclic-=3)==3 );
			CHECK( *(iCyclic-=3)==0 );
			CHECK( *(iCyclic-=4)==0 );
			CHECK( *(iCyclic-=4)==0 );
			CHECK( *(iCyclic-=5)==3 );
			CHECK( *(iCyclic-=5)==2 );
			CHECK( *(iCyclic-=5)==1 );
			CHECK( *(iCyclic-=5)==0 );
			CHECK( *(iCyclic-=5)==3 );
			CHECK( *(iCyclic-=18)==1 );
			CHECK( *(iCyclic-=18)==3 );
			CHECK( *(iCyclic-=18)==1 );
			CHECK( *(iCyclic-=18)==3 );
		}
	}
}
