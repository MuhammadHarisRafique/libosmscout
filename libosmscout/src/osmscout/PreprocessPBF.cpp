/*
  This source is part of the libosmscout library
  Copyright (C) 2010  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <osmscout/PreprocessPBF.h>

#include <osmscout/private/Config.h>

#include <cstdio>

// We should try to get rid of this!
#include <arpa/inet.h>

#if defined(HAVE_LIB_ZLIB)
  #include <zlib.h>
#endif

#include <osmscout/pbf/fileformat.pb.h>
#include <osmscout/pbf/osmformat.pb.h>

#include <osmscout/RawNode.h>
#include <osmscout/RawRelation.h>
#include <osmscout/RawWay.h>
#include <osmscout/Util.h>

#include <iostream>

#define NANO ( 1000.0 * 1000.0 * 1000.0 )

namespace osmscout {

  bool ReadBlockHeader(Progress& progress,
                       FILE* file,
                       PBF::BlockHeader& blockHeader)
  {
    char blockHeaderLength[4];

    if (fread(blockHeaderLength,sizeof(char),4,file)!=4) {
      progress.Error("Cannot read block header length!");
      return false;
    }

    // ugly!
    uint32_t length=ntohl(*((uint32_t*)&blockHeaderLength));

    // TODO: Check size

    char *buffer=new char[length];

    if (fread(buffer,sizeof(char),length,file)!=length) {
      progress.Error("Cannot read block header!");
      delete[] buffer;
      return false;
    }

    if (!blockHeader.ParseFromArray(buffer,length)) {
      progress.Error("Cannot parse block header!");
      delete[] buffer;
      return false;
    }

    delete[] buffer;

    return true;
  }

  bool ReadHeaderBlock(Progress& progress,
                       FILE* file,
                       const PBF::BlockHeader& blockHeader,
                       PBF::HeaderBlock& headerBlock)
  {
    PBF::Blob blob;

    uint32_t length = blockHeader.datasize();

    // TODO: Check size

    char *buffer=new char[length];

    if (fread(buffer,sizeof(char),length,file)!=length) {
      progress.Error("Cannot read blob!");
      delete[] buffer;
      return false;
    }

    if (!blob.ParseFromArray(buffer,length)) {
      progress.Error("Cannot parse blob!");
      delete[] buffer;
      return false;
    }

    delete [] buffer;

    if (blob.has_raw()) {
      length=blob.raw().length();
      buffer = new char[length];
      memcpy(buffer,blob.raw().data(),length);
    }
    else if (blob.has_zlib_data()){
#if defined(HAVE_LIB_ZLIB)
      length=blob.raw_size();
      buffer=new char[length];

      z_stream compressedStream;

      compressedStream.next_in=(unsigned char*)blob.zlib_data().data();
      compressedStream.avail_in=blob.zlib_data().size();
      compressedStream.next_out=(Bytef*)buffer;
      compressedStream.avail_out=length;
      compressedStream.zalloc=Z_NULL;
      compressedStream.zfree=Z_NULL;
      compressedStream.opaque=Z_NULL;

      if (inflateInit( &compressedStream)!=Z_OK) {
        progress.Error("Cannot decode zlib compressed blob data!");
        delete[] buffer;
        return false;
      }

      if (inflate(&compressedStream,Z_FINISH)!=Z_STREAM_END) {
        progress.Error("Cannot decode zlib compressed blob data!");
        delete[] buffer;
        return false;
      }

      if (inflateEnd(&compressedStream)!=Z_OK) {
        progress.Error("Cannot decode zlib compressed blob data!");
        delete[] buffer;
        return false;
      }
#else
      progress.Error("Data is zlib encoded but zlib support is not enabled!");
      return false;
#endif
    }
    else if (blob.has_bzip2_data()){
      progress.Error("Data is bzip2 encoded but bzip2 support is not enabled!");
      return false;
    }
    else if (blob.has_lzma_data()){
      progress.Error("Data is lzma encoded but lzma support is not enabled!");
      return false;
    }

    if (!headerBlock.ParseFromArray(buffer,length)) {
      progress.Error("Cannot parse header block!");
      delete[] buffer;
      return false;
    }

    delete[] buffer;

    return true;
  }

  bool ReadPrimitiveBlock(Progress& progress,
                          FILE* file,
                          const PBF::BlockHeader& blockHeader,
                          PBF::PrimitiveBlock& primitiveBlock)
  {
    PBF::Blob blob;

    uint32_t length = blockHeader.datasize();

    // TODO: Check size

    char *buffer=new char[length];

    if (fread(buffer,sizeof(char),length,file)!=length) {
      progress.Error("Cannot read blob!");
      delete[] buffer;
      return false;
    }

    if (!blob.ParseFromArray(buffer,length)) {
      progress.Error("Cannot parse blob!");
      delete[] buffer;
      return false;
    }

    delete [] buffer;

    if (blob.has_raw()) {
      length=blob.raw().length();
      buffer = new char[length];
      memcpy(buffer,blob.raw().data(),length);
    }
    else if (blob.has_zlib_data()){
#if defined(HAVE_LIB_ZLIB)
      length=blob.raw_size();
      buffer=new char[length];

      z_stream compressedStream;

      compressedStream.next_in=(unsigned char*)blob.zlib_data().data();
      compressedStream.avail_in=blob.zlib_data().size();
      compressedStream.next_out=(Bytef*)buffer;
      compressedStream.avail_out=length;
      compressedStream.zalloc=Z_NULL;
      compressedStream.zfree=Z_NULL;
      compressedStream.opaque=Z_NULL;

      if (inflateInit( &compressedStream)!=Z_OK) {
        progress.Error("Cannot decode zlib compressed blob data!");
        delete[] buffer;
        return false;
      }

      if (inflate(&compressedStream,Z_FINISH)!=Z_STREAM_END) {
        progress.Error("Cannot decode zlib compressed blob data!");
        delete[] buffer;
        return false;
      }

      if (inflateEnd(&compressedStream)!=Z_OK) {
        progress.Error("Cannot decode zlib compressed blob data!");
        delete[] buffer;
        return false;
      }
#else
      progress.Error("Data is zlib encoded but zlib support is not enabled!");
      return false;
#endif
    }
    else if (blob.has_bzip2_data()){
      progress.Error("Data is bzip2 encoded but bzip2 support is not enabled!");
      return false;
    }
    else if (blob.has_lzma_data()){
      progress.Error("Data is lzma encoded but lzma support is not enabled!");
      return false;
    }

    if (!primitiveBlock.ParseFromArray(buffer,length)) {
      progress.Error("Cannot parse primitive block!");
      delete[] buffer;
      return false;
    }

    delete[] buffer;

    return true;
  }

  std::string PreprocessPBF::GetDescription() const
  {
    return "PreprocessPBF";
  }

  bool PreprocessPBF::Import(const ImportParameter& parameter,
                             Progress& progress,
                             const TypeConfig& typeConfig)
  {
    FileWriter nodeWriter;
    FileWriter wayWriter;
    FileWriter relationWriter;

    uint32_t   nodeCount=0;
    uint32_t   wayCount=0;
    uint32_t   areaCount=0;
    uint32_t   relationCount=0;

    progress.SetAction(std::string("Parsing PBF file '")+parameter.GetMapfile()+"'");

    FILE* file;

    file=fopen(parameter.GetMapfile().c_str(),"rb");

    if (file==NULL) {
      progress.Error("Cannot open file!");
      return false;
    }

    nodeWriter.Open("rawnodes.dat");
    nodeWriter.Write(nodeCount);

    wayWriter.Open("rawways.dat");
    wayWriter.Write(wayCount+areaCount);

    relationWriter.Open("rawrels.dat");
    relationWriter.Write(relationCount);

    // BlockHeader

    PBF::BlockHeader blockHeader;

    if (!ReadBlockHeader(progress,file,blockHeader)) {
      fclose(file);
      return false;
    }

    if (blockHeader.type()!="OSMHeader") {
      progress.Error("File is not an OSM PBF file!");
      fclose(file);
      return false;
    }

    PBF::HeaderBlock headerBlock;

    if (!ReadHeaderBlock(progress,
        file,
        blockHeader,
        headerBlock)) {
      fclose(file);
      return false;
    }

    for (int i=0; i<headerBlock.required_features_size(); i++) {
      std::string feature=headerBlock.required_features(i);
      if (feature!="OsmSchema-V0.6" &&
          feature!="DenseNodes") {
        progress.Error(std::string("Unsupported feature '")+feature+"'");
        fclose(file);
        return false;
      }
    }

    RawNode     rawNode;
    RawWay      rawWay;
    RawRelation rawRel;

    rawNode.tags.reserve(20);

    rawWay.tags.reserve(20);
    rawWay.nodes.reserve(2000);

    rawRel.tags.reserve(20);
    rawRel.members.reserve(2000);

    while (true) {
      PBF::BlockHeader blockHeader;

      if (!ReadBlockHeader(progress,file,blockHeader)) {
        fclose(file);
        break;
      }

      if (blockHeader.type()!="OSMData") {
        progress.Error("File is not an OSM PBF file!");
        fclose(file);
        return false;
      }

      PBF::PrimitiveBlock block;

      if (!ReadPrimitiveBlock(progress,
                              file,
                              blockHeader,
                              block)) {
        fclose(file);
        return false;
      }

      for (int currentGroup=0;
           currentGroup<block.primitivegroup_size();
           currentGroup++) {
        const PBF::PrimitiveGroup &group=block.primitivegroup(currentGroup);

        if (group.nodes_size()>0) {
          for (int n=0; n<group.nodes_size(); n++) {
            const PBF::Node &inputNode=group.nodes(n);

            rawNode.id=inputNode.id();
            rawNode.lat=(inputNode.lat()*block.granularity()+block.lat_offset())/NANO;
            rawNode.lon=(inputNode.lon()*block.granularity()+block.lon_offset())/NANO;

            rawNode.tags.clear();

            for (int t=0; t<inputNode.keys_size(); t++) {
              TagId tagId=typeConfig.GetTagId(block.stringtable().s(inputNode.keys(t)).c_str());

              if (tagId==tagIgnore) {
                continue;
              }

              Tag tag;

              tag.key=tagId;
              tag.value=block.stringtable().s(inputNode.vals(t));

              rawNode.tags.push_back(tag);
            }
          }
        }
        else if (group.ways_size()>0) {
          for (int w=0; w<group.ways_size(); w++) {
            const PBF::Way &inputWay=group.ways(w);

            rawWay.nodes.clear();
            rawWay.tags.clear();

            rawWay.id=inputWay.id();

            for (int t=0; t<inputWay.keys_size(); t++) {
              TagId tagId=typeConfig.GetTagId(block.stringtable().s(inputWay.keys(t)).c_str());

              if (tagId==tagIgnore) {
                continue;
              }

              Tag tag;

              tag.key=tagId;
              tag.value=block.stringtable().s(inputWay.vals(t));

              rawWay.tags.push_back(tag);
            }

            long ref=0;
            for (int r=0; r<inputWay.refs_size();r++) {
              ref+=inputWay.refs(r);

              rawWay.nodes.push_back(ref);
            }

            TypeId                     areaType=typeIgnore;
            TypeId                     wayType=typeIgnore;
            std::vector<Tag>::iterator wayTag=rawWay.tags.end();
            std::vector<Tag>::iterator areaTag=rawWay.tags.end();

            typeConfig.GetWayAreaTypeId(rawWay.tags,wayTag,wayType,areaTag,areaType);

            if (areaType!=typeIgnore &&
                rawWay.nodes.size()>1 &&
                rawWay.nodes[0]==rawWay.nodes[rawWay.nodes.size()-1]) {
              rawWay.isArea=true;
              rawWay.tags.erase(areaTag);
            }
            else if (wayType!=typeIgnore) {
              rawWay.isArea=false;
              rawWay.tags.erase(wayTag);
            }
            else if (areaType==typeIgnore &&
                     wayType==typeIgnore) {
              rawWay.isArea=false;
              // Unidentified way
              /*
              std::cout << "--- " << id << std::endl;
              for (size_t tag=0; tag<tags.size(); tag++) {
                std::cout << tags[tag].key << "/" << tags[tag].value << std::endl;
              }*/
            }

            if (rawWay.isArea) {
              rawWay.type=areaType;
              areaCount++;
            }
            else {
              rawWay.type=wayType;
              wayCount++;
            }

            rawWay.Write(wayWriter);
          }
        }
        else if (group.relations_size()>0) {
          for (int r=0; r<group.relations_size(); r++) {
            const PBF::Relation &inputRelation=group.relations(r);

            rawRel.tags.clear();
            rawRel.members.clear();

            rawRel.id=inputRelation.id();

            for (int t=0; t<inputRelation.keys_size(); t++) {
              TagId tagId=typeConfig.GetTagId(block.stringtable().s(inputRelation.keys(t)).c_str());

              if (tagId==tagIgnore) {
                continue;
              }

              Tag tag;

              tag.key=tagId;
              tag.value=block.stringtable().s(inputRelation.vals(t));

              rawRel.tags.push_back(tag);
            }

            long ref=0;
            for (int r=0; r<inputRelation.types_size();r++) {
              RawRelation::Member member;

              switch (inputRelation.types(r)) {
              case PBF::Relation::NODE:
                member.type=RawRelation::memberNode;
                break;
              case PBF::Relation::WAY:
                member.type=RawRelation::memberWay;
                break;
              case PBF::Relation::RELATION:
                member.type=RawRelation::memberRelation;
                break;
              }

              ref+=inputRelation.memids(r);

              member.id=ref;
              member.role=block.stringtable().s(inputRelation.roles_sid(r));

              rawRel.members.push_back(member);
            }

            std::vector<Tag>::iterator tag;

            rawRel.type=typeIgnore;

            if (typeConfig.GetRelationTypeId(rawRel.tags,
                                             tag,
                                             rawRel.type))  {
              rawRel.tags.erase(tag);
            }

            rawRel.Write(relationWriter);

            relationCount++;
          }
        }
        else if (group.has_dense()) {
          const PBF::DenseNodes &dense=group.dense();
          unsigned long dId=0;
          double        dLat=0;
          double        dLon=0;
          int           t=0;

          for (int d=0; d<dense.id_size();d++) {
            dId+=dense.id(d);
            dLat+=dense.lat(d);
            dLon+=dense.lon(d);

            rawNode.id=dId;
            rawNode.lat=(dLat*block.granularity()+block.lat_offset())/NANO;
            rawNode.lon=(dLon*block.granularity()+block.lon_offset())/NANO;

            rawNode.tags.clear();

            while (true) {
              if (t>=dense.keys_vals_size()) {
                break;
              }

              if (dense.keys_vals(t)==0) {
                t++;
                break;
              }

              TagId tagId=typeConfig.GetTagId(block.stringtable().s(dense.keys_vals(t)).c_str());

              if (tagId==tagIgnore) {
                t+=2;
                continue;
              }

              Tag tag;

              tag.key=tagId;
              tag.value=block.stringtable().s(dense.keys_vals(t+1));

              t+=2;

              rawNode.tags.push_back(tag);
            }

            std::vector<Tag>::iterator tag;

            if (typeConfig.GetNodeTypeId(rawNode.tags,tag,rawNode.type))  {
              rawNode.tags.erase(tag);
            }

            rawNode.Write(nodeWriter);
            nodeCount++;
          }
        }
      }
    }

    nodeWriter.SetPos(0);
    nodeWriter.Write(nodeCount);

    wayWriter.SetPos(0);
    wayWriter.Write(wayCount+areaCount);

    relationWriter.SetPos(0);
    relationWriter.Write(relationCount);

    nodeWriter.Close();
    wayWriter.Close();
    relationWriter.Close();

    progress.Info(std::string("Nodes:          ")+NumberToString(nodeCount));
    progress.Info(std::string("Ways/Areas/Sum: ")+NumberToString(wayCount)+" "+
                  NumberToString(areaCount)+" "+
                  NumberToString(wayCount+areaCount));
    progress.Info(std::string("Relations:      ")+NumberToString(relationCount));

    return true;
  }
}
