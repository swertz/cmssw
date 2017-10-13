#include "CondCore/Utilities/interface/PayloadInspectorModule.h"
#include "CondCore/Utilities/interface/PayloadInspector.h"
#include "CondCore/CondDB/interface/Time.h"
#include "CondFormats/HLTObjects/interface/AlCaRecoTriggerBits.h"

#include <memory>
#include <sstream>
#include <iostream>
#include "TCanvas.h"
#include "TLatex.h"
#include "TLine.h"

namespace {
  
  /************************************************
    Display AlCaRecoTriggerBits mapping
  *************************************************/
  class AlCaRecoTriggerBits_Display: public cond::payloadInspector::PlotImage<AlCaRecoTriggerBits> {
  public:
    AlCaRecoTriggerBits_Display() : cond::payloadInspector::PlotImage<AlCaRecoTriggerBits>( "Table of AlCaRecoTriggerBits" ){
      setSingleIov( true );
    }

    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ) override{
      auto iov = iovs.front();
      std::shared_ptr<AlCaRecoTriggerBits> payload = fetchPayload( std::get<1>(iov) );

      std::string IOVsince  = std::to_string(std::get<0>(iov));

      
      // Get map of strings to concatenated list of names of HLT paths:
      typedef std::map<std::string, std::string> TriggerMap;
      const TriggerMap &triggerMap = payload->m_alcarecoToTrig;

      unsigned int mapsize = triggerMap.size();
      float pitch = 1./(mapsize*1.20);

      //std::cout<<"map size:" << mapsize << " pitch:" << pitch << "ratio: " << 1000./mapsize << std::endl;

      TCanvas canvas("AlCaRecoTriggerBits","AlCaRecoTriggerBits",2000,40*mapsize); 

      TLatex l;
      l.SetTextSize(pitch-0.002);

      // Draw the columns titles
      l.SetTextAlign(12);

      float y, x1, x2;
      y = 1.0; x1 = 0.02; x2 = x1+0.20;
	
      y -= pitch; 
      l.DrawLatexNDC(x1, y,"#scale[1.2]{Key}"); 
      l.DrawLatexNDC(x2, y,("#scale[1.2]{in IOV: "+IOVsince+"}").c_str()); 
      y -= 0.005;

      for(const auto &element : triggerMap){
	//std::cout<< element.first << " : " ;

	y -= pitch; l.DrawLatex(x1, y, element.first.c_str()); 

	std::map<int,std::string> output;
	int count=0;
	const std::vector<std::string> paths = payload->decompose(element.second);
	for (unsigned int iPath = 0; iPath < paths.size(); ++iPath) {
	  //std::cout << paths[iPath] << " ; " ;
	  output[count]+=paths[iPath];
	  output[count]+="; ";
	  if(output[count].length()>80) count++;
	}
      	
	for (unsigned int br=0; br<output.size();br++){
	  l.DrawLatexNDC(x2,y,("#color[2]{"+output[br]+"}").c_str());  
	  if(br!=output.size()-1) y-=pitch;
	}

	TLine *line = new TLine(gPad->GetUxmin(),y-(pitch/2.),gPad->GetUxmax(),y-(pitch/2.));
	line->Draw();

	//std::cout << std::endl;
      }

      std::string fileName(m_imageFileName);
      canvas.SaveAs(fileName.c_str());
      return true;
    }
  };

  /************************************************
    Compare AlCaRecoTriggerBits mapping
  *************************************************/
  class AlCaRecoTriggerBits_Compare: public cond::payloadInspector::PlotImage<AlCaRecoTriggerBits> {
  public:
    AlCaRecoTriggerBits_Compare() : cond::payloadInspector::PlotImage<AlCaRecoTriggerBits>( "Table of AlCaRecoTriggerBits" ){
      setSingleIov( false );
    }

    bool fill( const std::vector<std::tuple<cond::Time_t,cond::Hash> >& iovs ) override{

      std::vector<std::tuple<cond::Time_t,cond::Hash> > sorted_iovs = iovs;
      
      // make absolute sure the IOVs are sortd by since
      std::sort(begin(sorted_iovs), end(sorted_iovs), [](auto const &t1, auto const &t2) {
	  return std::get<0>(t1) < std::get<0>(t2);
	});
      
      auto firstiov  = sorted_iovs.front();
      auto lastiov   = sorted_iovs.back();
      
      std::shared_ptr<AlCaRecoTriggerBits> last_payload  = fetchPayload( std::get<1>(lastiov) );
      std::shared_ptr<AlCaRecoTriggerBits> first_payload = fetchPayload( std::get<1>(firstiov) );
      
      std::string lastIOVsince  = std::to_string(std::get<0>(lastiov));
      std::string firstIOVsince = std::to_string(std::get<0>(firstiov));
      
      // Get map of strings to concatenated list of names of HLT paths:
      typedef std::map<std::string, std::string> TriggerMap;
      const TriggerMap &first_triggerMap = first_payload->m_alcarecoToTrig;
      const TriggerMap &last_triggerMap  = last_payload->m_alcarecoToTrig;

      std::vector<std::string> first_keys, not_in_first_keys;
      std::vector<std::string> last_keys, not_in_last_keys;

      unsigned int f_mapsize = first_triggerMap.size();
      unsigned int l_mapsize = last_triggerMap.size();

      // fill the vector of first keys
      for (const auto& element : first_triggerMap){
	first_keys.push_back(element.first);
      }

      // fill the vector of last keys
      for (const auto& element : last_triggerMap){
	last_keys.push_back(element.first);
      }

      // find the elements not in common
      std::set_difference(first_keys.begin(),first_keys.end(),last_keys.begin(),last_keys.end(), 
			  std::inserter(not_in_last_keys, not_in_last_keys.begin()));
      
      std::set_difference(last_keys.begin(),last_keys.end(),first_keys.begin(),first_keys.end(), 
			  std::inserter(not_in_first_keys, not_in_first_keys.begin()));

      TCanvas canvas1("AlCaRecoTriggerBits0","AlCaRecoTriggerBits0",2000,1000); 

      TLatex l;
      l.SetTextSize(0.018);

      // Draw the columns titles
      l.SetTextAlign(12);

      float y, x1, x2, x3;
      y  = 1.0; 
      x1 = 0.02; 
      x2 = x1+0.15; 
      x3 = x2+0.35;

      y -= 0.017; 
      l.DrawLatexNDC(x1, y,"#scale[1.2]{Key}"); 
      l.DrawLatexNDC(x2, y,("#scale[1.2]{in IOV: "+firstIOVsince+"}").c_str()); 
      l.DrawLatexNDC(x3, y,("#scale[1.2]{in IOV: "+lastIOVsince+"}").c_str()); 
      y -= 0.005;

      // print the ones missing in the last key
      for(const auto& key : not_in_last_keys ) {
	//std::cout<< key ;
	y -= 0.017; l.DrawLatexNDC(x1, y,key.c_str()); 
	const std::vector<std::string> missing_in_last_paths = first_payload->decompose(first_triggerMap.at(key));
	
	std::map<int,std::string> output;
	int count=0;
	for (unsigned int iPath = 0; iPath < missing_in_last_paths.size(); ++iPath) {
	  //std::cout << missing_in_last_paths[iPath] << " ; " ;
	  output[count]+=missing_in_last_paths[iPath];
	  output[count]+=";";
	  if(output[count].length()>60) count++;
	}
	
	for (unsigned int br=0; br<output.size();br++){
	  l.DrawLatexNDC(x2,y,("#color[2]{"+output[br]+"}").c_str());  
	  if(br!=output.size()-1) y -=0.017;
	}
	//std::cout << " |||||| not in last";
	//std::cout << std::endl;

	TLine *line = new TLine(gPad->GetUxmin(),y-0.008,gPad->GetUxmax(),y-0.008);
	line->Draw();

      }
      

      // print the ones missing in the first key
      for(const auto& key : not_in_first_keys ) {
	//std::cout<< key ;
	y -= 0.017; l.DrawLatexNDC(x1, y,key.c_str()); 
	const std::vector<std::string> missing_in_first_paths = last_payload->decompose(last_triggerMap.at(key));

	//std::cout << " not in first ||||||";
	std::map<int,std::string> output;
	int count=0;
	for (unsigned int iPath = 0; iPath < missing_in_first_paths.size(); ++iPath) {
	  //std::cout << missing_in_first_paths[iPath] << " ; " ;
	  output[count]+=missing_in_first_paths[iPath];
	  output[count]+=";";
	  if(output[count].length()>60) count++;
	}

	for (unsigned int br=0; br<output.size();br++){
	  l.DrawLatexNDC(x3,y,("#color[4]{"+output[br]+"}").c_str());  	    
	  if(br!=output.size()-1) y -= 0.017;
	}
	//std::cout << std::endl;
	
	TLine *line2 = new TLine(gPad->GetUxmin(),y-0.008,gPad->GetUxmax(),y-0.008);
	line2->Draw();
      
      }

      for(const auto &element : first_triggerMap){

      	if(last_triggerMap.find(element.first)!=last_triggerMap.end()){

      	  auto lastElement = last_triggerMap.find(element.first);
	
      	  std::string output;
      	  const std::vector<std::string> first_paths = first_payload->decompose(element.second);
      	  const std::vector<std::string> last_paths  = last_payload->decompose(lastElement->second);

      	  std::vector<std::string> not_in_first;
      	  std::vector<std::string> not_in_last;

      	  std::set_difference(first_paths.begin(),first_paths.end(),last_paths.begin(),last_paths.end(), 
      			      std::inserter(not_in_last, not_in_last.begin()));

      	  std::set_difference(last_paths.begin(),last_paths.end(),first_paths.begin(),first_paths.end(), 
      	  		      std::inserter(not_in_first, not_in_first.begin()));

      	  if(not_in_last.size()!=0 || not_in_first.size()!=0) {

      	    //std::cout<< element.first << " : "  ;
      	    y -= 0.017; l.DrawLatexNDC(x1, y, element.first.c_str()); 
	    
      	    std::map<int,std::string> output; 
      	    int count(0);
      	    for (unsigned int iPath = 0; iPath < not_in_last.size(); ++iPath) {
      	      //std::cout << not_in_last[iPath] << " ; " ;
      	      output[count]+= not_in_last[iPath];
      	      output[count]+="; ";
      	      if(output[count].length()>60) count++;
      	    }

      	    for (unsigned int br=0; br<output.size();br++){
      	      l.DrawLatexNDC(x2,y-(br*0.017),("#color[6]{"+output[br]+"}").c_str());
      	    }  
      	    //std::cout << " ||||||";
	    
      	    output.clear();
      	    int count1=0;
      	    for (unsigned int jPath = 0; jPath < not_in_first.size(); ++jPath) {
      	      //std::cout << not_in_first[jPath] << " ; " ;
      	      output[count1]+= not_in_first[jPath];
      	      output[count1]+=";";
      	      if(output[count1].length()>60) count1++;
      	    }

      	    for (unsigned int br=0; br<output.size();br++){
      	      l.DrawLatexNDC(x3,y-(br*0.017),("#color[8]{"+output[br]+"}").c_str());  	    
      	    }

	    y-=std::max(count,count1)*0.017;
      	    TLine *line3 = new TLine(gPad->GetUxmin(),y-0.008,gPad->GetUxmax(),y-0.008);
      	    line3->Draw();
	    
	    // decrease the y position to the maximum of the two lists


      	    //std::cout << std::endl;
 
      	  } // close if there is at least a difference 
      	} // if there is a common key
      }
      //loop on the keys

      //canvas.SetCanvasSize(2000,(1-y)*1000);
      std::string fileName(m_imageFileName);
      canvas1.SaveAs(fileName.c_str());
      return true;
    }
  };


}


PAYLOAD_INSPECTOR_MODULE( AlCaRecoTriggerBits ){
  PAYLOAD_INSPECTOR_CLASS( AlCaRecoTriggerBits_Display );
  PAYLOAD_INSPECTOR_CLASS( AlCaRecoTriggerBits_Compare );
}
