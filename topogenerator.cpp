#include "topogenerator.h"
#include "graphmlReader.h"

#include <QTextStream>
#include <QDir>
#include <QList>
#include <QXmlStreamReader>
#include <QtMath>
#include <QCoreApplication>

topoGenerator::topoGenerator(QString srcPath, QString dstPath,
                             const ControllerDeploymentCostGeneator* CDCG,
                             const MaxLoadGeneator* MLG,
                             const AvgLoadGeneator* ALG,
                             const ConnectionCostGeneator* CCG,
                             const ControllerLoadGeneator* CLG,
                             const ControllerPlacementSettings* settings)
    :srcPath(srcPath), dstPath(dstPath), CDCG(CDCG), MLG(MLG), ALG(ALG), CCG(CCG), CLG(CLG), settings(settings){}

void topoGenerator::generate(){
    QDir dstDir(dstPath);
    ensureExp(dstDir.exists(),"topoGenrator error: dir " + dstPath + " doesn't exist!");
    QDir srcDir(srcPath);
    ensureExp(srcDir.exists(),"topoGenrator error: dir " + srcPath + " doesn't exist!");

    stop = false;

    QList<QString> inFilesList = srcDir.entryList(QStringList("*.graphml"), QDir::Files);
    try {
        emit minProgressChanged(0);
        emit maxProgressChanged(inFilesList.size());
        for(auto i = inFilesList.begin(); i!=inFilesList.end(); i++){
            try {
                emit valProgressChanged(i-inFilesList.begin());
                QCoreApplication::processEvents();
                if (stop)
                    throw StopProgram("Работа программы прервана пользователем");
                GRAPHMLReader reader(i->prepend(srcDir.absolutePath() + "/"), settings);

                generateTopo(reader, dstPath+"/"+reader.topoName + ".graphml");
            }
            catch (Exceptions ex){}
        }
    }
    catch (StopProgram sp){emit reject();}
    emit done();
}

void topoGenerator::generateTopo(const GRAPHMLReader& topo, QString dstPath){
    QFile file(dstPath);
    ensureExp(file.open(QFile::WriteOnly | QFile::Truncate),QString("Не получилось открыть файл %1").arg(dstPath));
    QTextStream fout(&file);

    int nodesNum = topo.getNodes()->size();
    int edgesNum = topo.getEdges()->size();

    fout<<"<?xml version=""1.0"" encoding=""utf-8""?>"<<endl;
    fout<<"<graphml>"<<endl;
    fout<<"  <key attr.name=""Latitude"" attr.type=""double"" for=""node"" id=""d1"" />"<<endl;
    fout<<"  <key attr.name=""Longitude"" attr.type=""double"" for=""node"" id=""d2"" />"<<endl;
    fout<<"  <key attr.name=""id"" attr.type=""int"" for=""node"" id=""d3"" />"<<endl;
    fout<<"  <key attr.name=""MaxSwitchLoad"" attr.type=""int"" for=""node"" id=""d4"" />"<<endl;
    fout<<"  <key attr.name=""ControllerLoad"" attr.type=""int"" for=""node"" id=""d5"" />"<<endl;
    fout<<"  <key attr.name=""cost"" attr.type=""int"" for=""node"" id=""d6"" />"<<endl;
    fout<<"  <key attr.name=""cost"" attr.type=""int"" for=""edge"" id=""d7"" />"<<endl;
    fout<<"  <key attr.name=""AvgSwitchLoad"" attr.type=""int"" for=""node"" id=""d8"" />"<<endl;
    fout<<"  <graph edgedefault=""undirected"">"<<endl;
    for (int node = 0; node < nodesNum; node++){
        fout<<"  <node id="""<<node<<""">"<<endl;
        fout<<"    <data key=""d1"">"<<topo.getNodes()->at(node).latitude<<"</data>"<<endl;
        fout<<"    <data key=""d2"">"<<topo.getNodes()->at(node).longitude<<"</data>"<<endl;
        fout<<"    <data key=""d3"">"<<topo.getNodes()->at(node).id<<"</data>"<<endl;
        fout<<"    <data key=""d4"">"<<MLG->generate()<<"</data>"<<endl;
        fout<<"    <data key=""d5"">"<<CLG->generate()<<"</data>"<<endl;
        fout<<"    <data key=""d6"">"<<CDCG->generate()<<"</data>"<<endl;
        fout<<"    <data key=""d8"">"<<ALG->generate()<<"</data>"<<endl;
        fout<<"  </node>"<<endl;
        if (node%5==0){
            QCoreApplication::processEvents();
            if (stop) throw StopProgram("stop pressed");
        }
    }
    for (int edge = 0; edge < edgesNum; edge++){
        fout<<"  <edge source="""<<topo.getEdges()->at(edge).srcId<<"""target="""<<topo.getEdges()->at(edge).dstId<<""">"<<endl;
        fout<<"    <data key=""d7"">"<<CCG->generate()<<"</data>"<<endl;
        fout<<"  </edge>"<<endl;
        if (edge%5==0){
            QCoreApplication::processEvents();
            if (stop) throw StopProgram("stop pressed");
        }
    }
    fout<<"</graphml>"<<endl;
}

int normalDistributionGenerator(int expected, double dispersion, int range){
    double pMinRange = qExp(-((double)(range*range)/2/dispersion)/qSqrt(2*M_PI*dispersion));
    double pMaxRange = 1/qSqrt(2*M_PI*dispersion);
    double p = (rand()%(int)floor(100000*pMaxRange - 100000*pMinRange + 1.5)+100000*pMinRange)/100000;
    int b = -2*expected;
    int c = expected*expected + 2*dispersion*qLn(p*qSqrt(2*M_PI*dispersion));
    double disc = b*b-4*c;
    ensureExp(disc>=0, "normalDistributionGenerator: discreminant < 0");
    if (rand()%2==0)
        return expected + qSqrt(disc)/2;
    else
        return expected - qSqrt(disc)/2;
}
