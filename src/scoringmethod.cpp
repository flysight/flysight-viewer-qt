/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

#include <QProgressDialog>

#include "mainwindow.h"
#include "scoringmethod.h"

ScoringMethod::ScoringMethod(QObject *parent) : QObject(parent)
{

}

void ScoringMethod::optimize(
        MainWindow *mainWindow,
        double windowBottom)
{
    const DataPoint &dp0 = mainWindow->interpolateDataT(0);

    // y = ax^2 + c
    const double m = 1 / mainWindow->maxLD();
    const double c = mainWindow->minDrag();
    const double a = m * m / (4 * c);

    const int workingSize    = 100;     // Working population
    const int keepSize       = 10;      // Number of elites to keep
    const int newSize        = 10;      // New genomes in first level
    const int numGenerations = 250;     // Generations per level of detail
    const int tournamentSize = 5;       // Number of individuals in a tournament
    const int mutationRate   = 100;     // Frequency of mutations
    const int truncationRate = 10;      // Frequency of truncations

    qsrand(QTime::currentTime().msec());

    const double dt = 0.25; // Time step (s)

    int kLim = 0;
    while (dt * (1 << kLim) < mainWindow->simulationTime())
    {
        ++kLim;
    }

    const int genomeSize = (1 << kLim) + 1;
    const int kMin = kLim - 4;
    const int kMax = kLim - 2;

    GenePool genePool;

    QProgressDialog progress("Initializing...",
                             "Abort",
                             0,
                             (kMax - kMin + 1) * numGenerations * workingSize + workingSize,
                             mainWindow);
    progress.setWindowModality(Qt::WindowModal);

    double maxScore = 0;
    bool abort = false;

    // Add new individuals
    for (int i = 0; i < workingSize; ++i)
    {
        progress.setValue(progress.value() + 1);
        if (progress.wasCanceled())
        {
            abort = true;
            break;
        }

        Genome g(genomeSize, kMin, mainWindow->minLift(), mainWindow->maxLift());
        const MainWindow::DataPoints result = g.simulate(dt, a, c, mainWindow->planformArea(), mainWindow->mass(), dp0, windowBottom);
        const double s = score(result);
        genePool.append(Score(s, g));

        maxScore = qMax(maxScore, s);
    }

    // Increasing levels of detail
    for (int k = kMin; k <= kMax && !abort; ++k)
    {
        // Generations
        for (int j = 0; j < numGenerations && !abort; ++j)
        {
            progress.setValue(progress.value() + keepSize);
            if (progress.wasCanceled())
            {
                abort = true;
                break;
            }

            // Sort gene pool by score
            qSort(genePool);

            // Elitism
            GenePool newGenePool = genePool.mid(0, keepSize);

            // Initialize score
            maxScore = 0;
            for (int i = 0; i < keepSize; ++i)
            {
                maxScore = qMax(maxScore, newGenePool[i].first);
            }

            // Add new individuals in first level
            for (int i = 0; k == kMin && i < newSize; ++i)
            {
                progress.setValue(progress.value() + 1);
                if (progress.wasCanceled())
                {
                    abort = true;
                    break;
                }

                Genome g(genomeSize, kMin, mainWindow->minLift(), mainWindow->maxLift());
                const MainWindow::DataPoints result = g.simulate(dt, a, c, mainWindow->planformArea(), mainWindow->mass(), dp0, windowBottom);
                const double s = score(result);
                newGenePool.append(Score(s, g));

                maxScore = qMax(maxScore, s);
            }

            // Tournament selection
            while (newGenePool.size() < workingSize)
            {
                progress.setValue(progress.value() + 1);
                if (progress.wasCanceled())
                {
                    abort = true;
                    break;
                }

                const Genome &p1 = selectGenome(genePool, tournamentSize);
                const Genome &p2 = selectGenome(genePool, tournamentSize);
                Genome g(p1, p2, k);

                if (qrand() % 100 < truncationRate)
                {
                    g.truncate(k);
                }
                if (qrand() % 100 < mutationRate)
                {
                    g.mutate(k, kMin, mainWindow->minLift(), mainWindow->maxLift());
                }

                const MainWindow::DataPoints result = g.simulate(dt, a, c, mainWindow->planformArea(), mainWindow->mass(), dp0, windowBottom);
                const double s = score(result);
                newGenePool.append(Score(s, g));

                maxScore = qMax(maxScore, s);
            }

            genePool = newGenePool;

            // Show best score in progress dialog
            QString labelText = scoreAsText(maxScore);
            progress.setLabelText(QString("Optimizing (best score is ") +
                                  labelText +
                                  QString(")..."));
        }
    }

    progress.setValue((kMax - kMin + 1) * numGenerations * workingSize + workingSize);

    // Sort gene pool by score
    qSort(genePool);

    // Keep most fit individual
    mainWindow->setOptimal(genePool[0].second.simulate(dt, a, c, mainWindow->planformArea(), mainWindow->mass(), dp0, windowBottom));
}

const Genome &ScoringMethod::selectGenome(
        const GenePool &genePool,
        const int tournamentSize)
{
    int jMax;
    double sMax;
    bool first = true;

    for (int i = 0; i < tournamentSize; ++i)
    {
        const int j = qrand() % genePool.size();
        if (first || genePool[j].first > sMax)
        {
            jMax = j;
            sMax = genePool[j].first;
            first = false;
        }
    }

    return genePool[jMax].second;
}
