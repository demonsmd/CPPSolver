#ifndef TOPOGENERATOR_H
#define TOPOGENERATOR_H
#include "topoelements.h"
#include "graphmlReader.h"

int normalDistributionGenerator(int expected, double dispersion, int range);

//======================= ControllerDeploymentCostGeneator =========================
class ControllerDeploymentCostGeneator
{
public:
    ControllerDeploymentCostGeneator(){}
    virtual ~ControllerDeploymentCostGeneator(){}
    virtual int generate()  const  = 0;
};

class NormalDistributedControllerDeploymentCostGeneator: public ControllerDeploymentCostGeneator
{
    int expected;
    double dispersion;
    int range;
public:
    NormalDistributedControllerDeploymentCostGeneator(int expected, double dispersion, int range)
        :expected(expected), dispersion(dispersion), range(range){}
    ~NormalDistributedControllerDeploymentCostGeneator(){}

    virtual int generate() const {return normalDistributionGenerator(expected, dispersion, range);}
};

class ConstControllerDeploymentCostGeneator: public ControllerDeploymentCostGeneator
{
    int value;
public:
    ConstControllerDeploymentCostGeneator(int value): value(value){}
    ~ConstControllerDeploymentCostGeneator(){}
    virtual int generate() const {return value;}
};

//======================= connectionCostGeneator =========================
class ConnectionCostGeneator
{
public:
    ConnectionCostGeneator(){}
    virtual ~ConnectionCostGeneator(){}
    virtual int generate()  const  = 0;
};

class ConstConnectionCostGeneator: public ConnectionCostGeneator
{
    int value;
public:
    ConstConnectionCostGeneator(int value): value(value){}
    ~ConstConnectionCostGeneator(){}
    virtual int generate() const {return value;}
};

class LatencyDependentConnectionCostGeneator: public ConnectionCostGeneator
{
    int value;
public:
    LatencyDependentConnectionCostGeneator(int value): value(value){}
    ~LatencyDependentConnectionCostGeneator(){}
    virtual int generate() const {return value;}
};

//======================= MaxLoadGeneator =========================
class MaxLoadGeneator
{
public:
    virtual ~MaxLoadGeneator(){}
    MaxLoadGeneator(){}
    virtual int generate() const = 0;
};

class NormalDistributedMaxLoadGeneator: public MaxLoadGeneator
{
    int expected;
    double dispersion;
    int range;
public:
    NormalDistributedMaxLoadGeneator(int expected, double dispersion, int range)
            :expected(expected), dispersion(dispersion), range(range){}
    ~NormalDistributedMaxLoadGeneator(){}

    virtual int generate() const {return normalDistributionGenerator(expected, dispersion, range);}
};

class ConstMaxLoadGeneator: public MaxLoadGeneator
{
    int value;
public:
    ConstMaxLoadGeneator(int value): value(value){}
    ~ConstMaxLoadGeneator(){}
    virtual int generate() const {return value;}
};

//======================= AvgLoadGeneator =========================
class AvgLoadGeneator
{
public:
    virtual ~AvgLoadGeneator(){}
    AvgLoadGeneator(){}
    virtual int generate() const = 0;
};

class NormalDistributedAvgLoadGeneator: public AvgLoadGeneator
{
    int expected;
    double dispersion;
    int range;
public:
    NormalDistributedAvgLoadGeneator(int expected, double dispersion, int range)
        :expected(expected), dispersion(dispersion), range(range){}
    ~NormalDistributedAvgLoadGeneator(){}

    virtual int generate() const {return normalDistributionGenerator(expected, dispersion, range);}
};

class ConstAvgLoadGeneator: public AvgLoadGeneator
{
    int value;
public:
    ConstAvgLoadGeneator(int value): value(value){}
    ~ConstAvgLoadGeneator(){}

    virtual int generate() const {return value;}
};

class MaxLoadLinearDependedAvgLoadGeneator: public AvgLoadGeneator
{
    MaxLoadGeneator* maxGenerator;
    int value;
public:
    MaxLoadLinearDependedAvgLoadGeneator(MaxLoadGeneator* maxGenerator, int value)
        :maxGenerator(maxGenerator), value(value){}
    ~MaxLoadLinearDependedAvgLoadGeneator(){}

    virtual int generate() const {return maxGenerator->generate()*value/100;}
};

//======================= ControllerLoadCostGeneator =========================
class ControllerLoadCostGeneator
{
public:
    virtual ~ControllerLoadCostGeneator(){}
    ControllerLoadCostGeneator(){}
    virtual int generate() const = 0;
};

class ConstControllerLoadCostGeneator: public ControllerLoadCostGeneator
{
    int value;
public:
    ConstControllerLoadCostGeneator(int value): value(value){}
     ~ConstControllerLoadCostGeneator(){}

    virtual int generate() const {return value;}
};

class NormalDistributedControllerLoadCostGeneator: public ControllerLoadCostGeneator
{
    int expected;
    double dispersion;
    int range;
public:
    NormalDistributedControllerLoadCostGeneator(int expected, double dispersion, int range)
        :expected(expected), dispersion(dispersion), range(range){}
    ~NormalDistributedControllerLoadCostGeneator(){}

    virtual int generate() const {return normalDistributionGenerator(expected, dispersion, range);}
};

//======================= ControllerLoadGeneator =========================
class ControllerLoadGeneator
{
public:
    ControllerLoadGeneator(){}
    virtual ~ControllerLoadGeneator(){}
    virtual int generate() const = 0;
};

class ConstControllerLoadGeneator: public ControllerLoadGeneator
{
    int value;
public:
    ConstControllerLoadGeneator(int value):value(value){}
    ~ConstControllerLoadGeneator(){}
    virtual int generate() const {return value;}
};



// ==================== генератор входных топологий ====================
class topoGenerator : public QObject
{
    Q_OBJECT

public:
    topoGenerator(QString srcPath, QString dstPath,
                  const ControllerDeploymentCostGeneator* CDCG,
                  const MaxLoadGeneator* MLG,
                  const AvgLoadGeneator* ALG,
                  const ConnectionCostGeneator* CCG,
                  const ControllerLoadGeneator* CLG,
                  const ControllerPlacementSettings* settings);
    void generate();

private:
    QString srcPath;
    QString dstPath;
    const ControllerDeploymentCostGeneator* CDCG;
    const MaxLoadGeneator* MLG;
    const AvgLoadGeneator* ALG;
    const ConnectionCostGeneator* CCG;
    const ControllerLoadGeneator* CLG;
    bool stop;
    const ControllerPlacementSettings* settings;

    void generateTopo(const GRAPHMLReader& topo, QString dstPath);

signals:
    void maxProgressChanged(int val);
    void minProgressChanged(int val);
    void valProgressChanged(int val);
    void reject();
    void done();

public slots:
    void cancel(){stop = true; emit reject();}

};

#endif // TOPOGENERATOR_H
