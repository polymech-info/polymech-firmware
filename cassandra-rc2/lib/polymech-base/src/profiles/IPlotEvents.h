#ifndef IPLOT_EVENTS_H
#define IPLOT_EVENTS_H

class PlotBase;

/**
 * @brief An interface for a delegate to receive events from PlotBase derivatives.
 *
 * This allows the plot objects (SignalPlot, TemperatureProfile) to notify an
 * owner or manager of state changes without requiring a direct dependency on the
 * concrete class of that manager (e.g., PHApp).
 */
class IPlotEvents
{
public:
    virtual ~IPlotEvents() = default;

    virtual void onTemperatureProfileStarted(PlotBase *profile) {}
    virtual void onTemperatureProfileStopped(PlotBase *profile) {}
    virtual void onTemperatureProfilePaused(PlotBase *profile) {}
    virtual void onTemperatureProfileResumed(PlotBase *profile) {}
    virtual void onTemperatureProfileFinished(PlotBase *profile) {}
    virtual void onTemperatureProfileReady(PlotBase *profile) {}

    virtual void onPressureProfileStarted(PlotBase *profile) {}
    virtual void onPressureProfileStopped(PlotBase *profile) {}
    virtual void onPressureProfilePaused(PlotBase *profile) {}
    virtual void onPressureProfileResumed(PlotBase *profile) {}
    virtual void onPressureProfileFinished(PlotBase *profile) {}

    virtual void onSignalPlotStarted(PlotBase *plot) {}
    virtual void onSignalPlotStopped(PlotBase *plot) {}
    virtual void onSignalPlotPaused(PlotBase *plot) {}
    virtual void onSignalPlotResumed(PlotBase *plot) {}
    virtual void onSignalPlotFinished(PlotBase *plot) {}
};

#endif // IPLOT_EVENTS_H