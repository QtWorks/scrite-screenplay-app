/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import Scrite 1.0

// For use from within StructureView only!!!!
// Because we assume the existence of 'annotation' and 'annotationGripLoader'
// in the current context
Rectangle {
    id: annotationItem
    x: annotation.geometry.x
    y: annotation.geometry.y
    width: annotation.geometry.width
    height: annotation.geometry.height
    color: annotation.attributes.fillBackground ? (annotation.attributes.color ? annotation.attributes.color : annotation.attributes.backgroundColor)  : Qt.rgba(0,0,0,0)
    border {
        width: annotation.attributes.borderWidth
        color: annotation.attributes.borderColor
    }
    opacity: annotation.attributes.opacity / 100

    function grip() {
        annotationGripLoader.annotationItem = annotationItem
        annotationGripLoader.annotation = annotation
    }

    MouseArea {
        anchors.fill: parent
        enabled: annotationGripLoader.annotationItem !== annotationItem
        onClicked: parent.grip()
    }

    Component.onCompleted: {
        if(annotation.objectName === "ica") {
            Qt.callLater(grip)
            annotation.objectName = ""
        }
    }
}