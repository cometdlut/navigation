/**
 * Copyright (C) 2013 TomTom International B.V.
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.genivi.trafficincidentsservice.textgenerator.basictextgenerator;

import java.util.ListResourceBundle;

public class TextParts_en extends ListResourceBundle {
  public Object[][] getContents() {
    return contents;
  }

  private Object[][] contents = {
      {"locationTemplate", "On the {0} from {1} to {2} between {3} and {4}: "},
      {"locationTemplateShort", "On the {0} between {1} and {2}: "},
      {"averageSpeedAbsoluteTemplate", " with an average driving speed of {0,number} {1}"},
      {"speedLimitTemplate", " with a speedlimit of {0,number} {1}"},
      {"expectedSpeedTemplate", " with an expected speed of {0,number} {1}"},
      {"lengthAffectedTemplate", " over a length of {0,number} {1}"},
      {"delayTemplate", " resulting in a delay of {0} minutes"},
      {"fromPart", " from "},
      {"tillPart", " till "},
      {"tendencyPart", " tendency is "},
      {"causedByPart", " caused by"},
      {"advicePart", "advice"},
      {"laneRestrictionsPart", " lane restrictions "},
      {"unverifiedInformationPart", " (unverified information)"},
      {"applicabilityPart", " This information is only applicable for:"},
      {"tomorrow", "tomorrow "},
      {"tonnes", "tonnes"},
      {"pounds", "pound"},
      {"foot", "feet"}
  };
}